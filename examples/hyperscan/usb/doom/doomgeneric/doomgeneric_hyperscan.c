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


void DG_DrawFrame(void)
{
	uint16_t i = 0;
    //int w      = DOOMGENERIC_RESX;
    //int h      = DOOMGENERIC_RESY;
  
    if (!FrameBuffer) return;

	hs_controller_read();
	
	if((controller[hs_controller_1].input.select)) {
		*(volatile unsigned int *)0x88210000 = (1<<6);		
	}
	
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

void DG_SleepMs(uint32_t ms)
{
}

volatile uint32_t ticks = 0;

uint32_t DG_GetTicksMs()
{
    return ticks+=12; //get_uptime_ms();
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

void patch_interrupt(){
	
		// Change exception vector base to 0xA0090000
	asm("la    	r4, 0xa0000000");
	asm("mtcr    r4, cr3");
	asm("nop");
	asm("nop");
	asm("nop");
	asm("nop");
	asm("nop");
	
	// Patch in vblank handler to count ticks
	volatile unsigned int *excvec_old_vblank = (volatile unsigned int *)0x800902D4;
	volatile unsigned int *excvec_new_vblank = (volatile unsigned int *)0x800002D4;

	volatile unsigned int *excvec_old_vblankc = (volatile unsigned int *)0xA00902D4;
	volatile unsigned int *excvec_new_vblankc = (volatile unsigned int *)0xA00002D4;
	
	volatile unsigned int *excvec_old_vblank1 = (volatile unsigned int *)0x800902D0;
	volatile unsigned int *excvec_new_vblank1 = (volatile unsigned int *)0x800002D0;

	volatile unsigned int *excvec_old_vblank1c = (volatile unsigned int *)0xA00902D0;
	volatile unsigned int *excvec_new_vblank1c = (volatile unsigned int *)0xA00002D0;

	//asm("li r4, 0x0");
	//asm("mtcr r4, cr0");
	//asm("nop");
	//asm("nop");
	//asm("nop");
	//asm("nop");
	//asm("nop");
	
	*excvec_new_vblank = *excvec_old_vblank;
	*excvec_new_vblankc = *excvec_old_vblankc;
	*excvec_new_vblank1 = *excvec_old_vblank1;
	*excvec_new_vblank1c = *excvec_old_vblank1c;

	*(volatile unsigned int *)0x88010090 = 0x5A;
	*(volatile unsigned int *)0x88010080 = 0x01;
	*(volatile unsigned int *)0x8809003C = 0x01;
		
	//asm("li r4, 0x1");
	//asm("mtcr r4, cr0");
	//asm("nop");
	//asm("nop");
	//asm("nop");
	//asm("nop");
	//asm("nop");
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
