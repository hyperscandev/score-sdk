//doomgeneric for Mattel HyperScan

#include "doomkeys.h"
#include "m_argv.h"
#include "doomgeneric.h"

#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>

#include "tv/tv.h"
#include "score7_registers.h"
#include "score7_constants.h"
#include "hyperscan/hs_controller/hs_controller.h"
#include "hyperscan/fatfs/ff.h"

#define FRAMEBUFFER_ADDR 0xA0500000
#define FrameBuffer ((volatile uint16_t*)FRAMEBUFFER_ADDR)

#define KEYQUEUE_SIZE 16

static unsigned short s_KeyQueue[KEYQUEUE_SIZE];
static unsigned int s_KeyQueueWriteIndex = 0;
static unsigned int s_KeyQueueReadIndex = 0;

static unsigned int s_PositionX = 0;
static unsigned int s_PositionY = 0;

void _unlink_r(){
	printf("_unlink_r called\n");
}

void _link_r(){
	printf("_link_r called\n");
}

static unsigned char convertToDoomKey(unsigned int key)
{
	return key;
}

static void addKeyToQueue(int pressed, unsigned int keyCode)
{
	unsigned char key = convertToDoomKey(keyCode);

	unsigned short keyData = (pressed << 8) | key;

	s_KeyQueue[s_KeyQueueWriteIndex] = keyData;
	s_KeyQueueWriteIndex++;
	s_KeyQueueWriteIndex %= KEYQUEUE_SIZE;
}

void DG_Init()
{

	/*
	Set TV output up with RGB565 color scheme and make set all framebuffers
	to stupid framebuffer address, TV_Init will select the first framebuffer
	as default.
	*/
	tv_init(RESOLUTION_320_240, COLOR_RGB565, FRAMEBUFFER_ADDR, FRAMEBUFFER_ADDR, FRAMEBUFFER_ADDR);

    int argPosX = 0;
    int argPosY = 0;

    argPosX = M_CheckParmWithArgs("-posx", 1);
    if (argPosX > 0)
    {
        sscanf(myargv[argPosX + 1], "%d", &s_PositionX);
    }

    argPosY = M_CheckParmWithArgs("-posy", 1);
    if (argPosY > 0)
    {
        sscanf(myargv[argPosY + 1], "%d", &s_PositionY);
    }
}

#include <stdint.h>
#include <string.h>
#include <stdbool.h>

// (Adjust these if your actual values differ.)
#define SRC_W   320
#define SRC_H   200
#define DST_W   320
#define DST_H   480

//------------------------------------------------------------------------
// 1) We’ll precompute, once, for each dst-row v=0..DST_H-1, the fixed-point
//    source position P = v*(SRC_H-1)/(DST_H-1) in 16.16
//    form, so that on every frame we just look up “src_y = P>>16” and
//    “frac = P & 0xFFFF”. This gives us smooth linear interpolation.
//------------------------------------------------------------------------
static bool   _fpos_inited = false;
static uint32_t fpos[DST_H];  // fixed-point (16.16) source position per dst-row
static uint16_t fsrc_y[DST_H];
static uint16_t ffrac[DST_H];

static void init_fpos(void)
{
    // We want P[v] = (v * (SRC_H-1) * 65536) / (DST_H-1)
    // store it in 16.16, so that:
    //    src_y   = P[v] >> 16
    //    frac16  = P[v] & 0xFFFF
    //
    // Note: We use (SRC_H-1)/(DST_H-1) so that the top row (v=0) maps to
    //       exactly row 0 in the source, and bottom row (v=DST_H-1) maps
    //       to exactly row SRC_H-1. That way we don’t accidentally read
    //       “SRC_H” out of range when v=DST_H-1.
    //
    const uint32_t numer = (uint32_t)(SRC_H - 1) * 65536u;
    const uint32_t denom = (uint32_t)(DST_H - 1);

	int v = 0;
    for (v = 0; v < DST_H; v++) {
        uint32_t P = (uint32_t)((uint64_t)numer * (uint64_t)v / denom);
        fpos[v]    = P;
        fsrc_y[v]  = (uint16_t)(P >> 16);         // integer row
        ffrac[v]   = (uint16_t)(P & 0xFFFFu);     // fractional a in [0..65535]
    }
    _fpos_inited = true;
}

//------------------------------------------------------------------------
// 2) Blending helper: Given two 5-6-5 pixels p0, p1 and a fraction a?[0..65535],
//    return ((1-a)*p0 + a*p1) channel-by-channel. We extract R/G/B, blend,
//    and re-pack into 5-6-5. Since src is 320×200 and dst is 320×480, a
//    per-pixel multiply is quite cheap vs. copying entire rows of blocks.
//------------------------------------------------------------------------
static inline uint16_t blend_565(uint16_t p0, uint16_t p1, uint16_t alpha16)
{
    // Extract (R,G,B) from each 5-6-5:
    uint32_t r0 = (p0 >> 11) & 0x1F;
    uint32_t g0 = (p0 >> 5)  & 0x3F;
    uint32_t b0 = (p0      ) & 0x1F;

    uint32_t r1 = (p1 >> 11) & 0x1F;
    uint32_t g1 = (p1 >> 5)  & 0x3F;
    uint32_t b1 = (p1      ) & 0x1F;

    uint32_t inv = 65536u - (uint32_t)alpha16;  // (1-a) in 16.16
    uint32_t r = (r0 * inv + r1 * (uint32_t)alpha16) >> 16;
    uint32_t g = (g0 * inv + g1 * (uint32_t)alpha16) >> 16;
    uint32_t b = (b0 * inv + b1 * (uint32_t)alpha16) >> 16;

    return (uint16_t)((r << 11) | (g << 5) | b);
}

//------------------------------------------------------------------------
// 3) The “smooth” DG_DrawFrame(): for each dst row, look up src_y and frac,
//    then for each x in the computed horizontal overlap region, fetch
//    src0 = screen[src_y][x], src1 = screen[src_y+1][x], blend, and write.
//------------------------------------------------------------------------
void DG_DrawFrame(void)
{
    if (!FrameBuffer || !DG_ScreenBuffer)
        return;

    if (!_fpos_inited) {
        init_fpos();
    }

    //
    // 1) figure out valid vertical range:  dy such that
    //    0 <= (v = dy - s_PositionY) < DST_H
    //
    int dy0 =  s_PositionY;
    int dy1 =  s_PositionY + DST_H - 1;
    if (dy0 < 0)         dy0 = 0;
    if (dy1 > DST_H - 1) dy1 = DST_H - 1;
    if (dy0 > dy1)       return;  // completely offscreen

    //
    // 2) figure out valid horizontal overlap:
    //    dst_x_min = max(0, sX)
    //    dst_x_max = min(DST_W-1, sX + SRC_W - 1)
    //
    int dst_x_min = s_PositionX;
    if (dst_x_min < 0)               dst_x_min = 0;
    if (dst_x_min > DST_W - SRC_W)   dst_x_min = DST_W;  // no overlap

    int dst_x_max = s_PositionX + (SRC_W - 1);
    if (dst_x_max < 0)               dst_x_max = -1;     // no overlap
    if (dst_x_max > DST_W - 1)       dst_x_max = DST_W - 1;

    if (dst_x_min > dst_x_max) {
        return;  // no horizontal overlap
    }

    int copy_w     = dst_x_max - dst_x_min + 1;  // # of pixels to process
    int src_x_base = dst_x_min - s_PositionX;    // where in source each row starts

    //
    // 3) For each dst-row dy = dy0..dy1:
    //      v     = dy - s_PositionY  (0..DST_H-1)
    //      src_y = fsrc_y[v]
    //      a     = ffrac[v]          (0..65535)
    //      next  = min(src_y+1, SRC_H-1)
    //    Then for each x=0..copy_w-1:
    //      S0 = DG_ScreenBuffer[src_y * SRC_W + (src_x_base + x)]
    //      S1 = DG_ScreenBuffer[next   * SRC_W + (src_x_base + x)]
    //      DST = blend_565(S0, S1, a)
    //
    int dy = 0;
    for (dy = dy0; dy <= dy1; dy++) {
        int v       = dy - s_PositionY;     // 0..DST_H-1
        uint16_t sy = fsrc_y[v];            // integer source row
        uint16_t alpha = ffrac[v];          // 0..65535
        uint16_t sy2 = (sy < (uint16_t)(SRC_H - 1)) ? (sy + 1) : sy;

        uint16_t *dst_row = FrameBuffer + (dy * DST_W) + dst_x_min;
        uint16_t *srow0   = DG_ScreenBuffer + (sy  * SRC_W) + src_x_base;
        uint16_t *srow1   = DG_ScreenBuffer + (sy2 * SRC_W) + src_x_base;
		int x = 0;
        for (x = 0; x < copy_w; x++) {
            uint16_t p0 = srow0[x];
            uint16_t p1 = srow1[x];
            dst_row[x]  = blend_565(p0, p1, alpha);
        }
    }
}


/*
void DG_DrawFrame(void)
{
	uint16_t i = 0;
    //int w      = DOOMGENERIC_RESX;
    //int h      = DOOMGENERIC_RESY;
  
    if (!FrameBuffer) return;

	hs_controller_read();
	
	if((controller[hs_controller_1].input.select) & (controller[hs_controller_1].input.b)){
		tv_clearscreen(FrameBuffer);
		s_PositionX += 2;
		printf("s_PositionX: %d\n", s_PositionX);
	}
	
	if((controller[hs_controller_1].input.select) & (controller[hs_controller_1].input.y)){
		tv_clearscreen(FrameBuffer);
		s_PositionX -= 2;
		printf("s_PositionX: %d\n", s_PositionX);
	}
		
	if((controller[hs_controller_1].input.select) & (controller[hs_controller_1].input.r)){
		tv_clearscreen(FrameBuffer);
		s_PositionY -= 2;
		printf("s_PositionY: %d\n", s_PositionY);
	}
	
	if((controller[hs_controller_1].input.select) & (controller[hs_controller_1].input.g)){
		tv_clearscreen(FrameBuffer);
		s_PositionY += 2;
		printf("s_PositionY: %d\n", s_PositionY);
	}

	for (i = 0; i < DOOMGENERIC_RESY * 2; ++i)
	{
		memcpy(FrameBuffer + s_PositionX + (i + s_PositionY) * 200, DG_ScreenBuffer + i * DOOMGENERIC_RESX, DOOMGENERIC_RESX);
    }
    //for (row = 0; row < h; row++)
    //{
        // Source: advance y rows in the doom buffer
    //    const void *src = DG_ScreenBuffer + row * w;

        // Dest: (init_y + row) scanlines down, then init_x pixels in
    //    size_t pixelIndex = (size_t)(s_PositionY + row) * DOOMGENERIC_RESX + s_PositionX;
    //    void *dst = (uint8_t*)FrameBuffer + pixelIndex * 2;

        // Copy exactly w pixels (w*bpp bytes) from src?dst
    //    memcpy(dst, src, w * 2);
    //}

    //handleKeyInput();
}
*/
void DG_SleepMs(uint32_t ms)
{
}

volatile uint32_t ticks = 0;

uint32_t DG_GetTicksMs()
{
    return ticks+=5; //get_uptime_ms();
}

int DG_GetKey(int* pressed, unsigned char* doomKey)
{
    if (s_KeyQueueReadIndex == s_KeyQueueWriteIndex)
    {
        //key queue is empty

        return 0;
    }
    else
    {
        unsigned short keyData = s_KeyQueue[s_KeyQueueReadIndex];
        s_KeyQueueReadIndex++;
        s_KeyQueueReadIndex %= KEYQUEUE_SIZE;

        *pressed = keyData >> 8;
        *doomKey = keyData & 0xFF;

        return 1;
    }
}

void DG_SetWindowTitle(const char * title)
{
}

int main(int argc, char **argv)
{
	hs_controller_init();
	
	ticks = 0;

	//patch_interrupt();
	
	FATFS fs0;
	f_mount(&fs0, "0:", 1);
	
	char *fake_argv[] = {"HYPER.EXE", "-iwad", "doom1.wad", "-nomouse", "-nosound", "-scaling", "1", NULL};
	
	int fake_argc = sizeof(fake_argv) / sizeof(fake_argv[0]) - 1;
	
	doomgeneric_Create(fake_argc, fake_argv);

	while(1) {
        doomgeneric_Tick();
    }
    
    return 0;
}
