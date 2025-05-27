/*

This is part of the Mattel HyperScan SDK by ppcasm (ppcasm@gmail.com)

usbloader
 
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "mp3drv/mp3.h"
#include "mp3drv/mp3drv.h"

#include "tv/tv.h"
#include "irq/interrupts.h"
#include "hyperscan/fatfs/ff.h"
#include "score7_registers.h"
#include "score7_constants.h"
#include "hyperscan/hs_controller/hs_controller.h"

#define FRAMEBUFFER_ADDRESS 0xA0400000
#define FRAMEBUFFER_WIDTH   640
#define FRAMEBUFFER_HEIGHT  480

#define LOAD_ADDRESS 0xA00901FC
#define ENTRY_ADDRESS 0xA0091000

void draw_pixel(unsigned short* framebuffer, int x, int y, unsigned short color) {
    if (x >= 0 && x < FRAMEBUFFER_WIDTH && y >= 0 && y < FRAMEBUFFER_HEIGHT) {
        framebuffer[y * FRAMEBUFFER_WIDTH + x] = color;
    }
}

void box(unsigned short* framebuffer, int x, int y, int width, int height, int radius, unsigned short color) {

	int i = 0;
	int dx = 0;
	int dy = 0;
    int x0 = x;
    int y0 = y;
    int x1 = x + width - 1;
    int y1 = y + height - 1;

    // Draw top and bottom lines
    for (i = x0 + radius; i <= x1 - radius; i++) {
        draw_pixel(framebuffer, i, y0, color);
        draw_pixel(framebuffer, i, y1, color);
    }

    // Draw left and right lines
    for (i = y0 + radius; i <= y1 - radius; i++) {
        draw_pixel(framebuffer, x0, i, color);
        draw_pixel(framebuffer, x1, i, color);
    }

    // Draw rounded corners
    for (dy = -radius; dy <= radius; dy++) {
        for (dx = -radius; dx <= radius; dx++) {
            if (dx*dx + dy*dy <= radius*radius) {
                draw_pixel(framebuffer, x0 + radius + dx, y0 + radius + dy, color);
                draw_pixel(framebuffer, x1 - radius + dx, y0 + radius + dy, color);
                draw_pixel(framebuffer, x0 + radius + dx, y1 - radius + dy, color);
                draw_pixel(framebuffer, x1 - radius + dx, y1 - radius + dy, color);
            }
        }
    }
}

void menu_border(unsigned short* framebuffer, int x, int y, int width, int height, int radius, int thickness, unsigned short color) {
	int t = 0;
    for (t = 0; t < thickness; t++) {
        box(framebuffer, x + t, y + t, width - t * 2, height - t * 2, radius, color);
    }
}

void dac_Init(){
	
	// Patch the excvec to allow fixed exc handler
	// to use our DAC ISR while still keeping firmware
	// in control of the exc vec
	unsigned int *excvec_old_dac = (unsigned int *)0xA0E002FC;
	unsigned int *excvec_new_dac = (unsigned int *)0xA00002FC;
	
	asm("li r4, 0x0");
	asm("mtcr r4, cr0");
	asm("nop");
	asm("nop");
	asm("nop");
	asm("nop");
	asm("nop");
	
	*excvec_new_dac = *excvec_old_dac;	

	asm("li r4, 0x1");
	asm("mtcr r4, cr0");
	asm("nop");
	asm("nop");
	asm("nop");
	asm("nop");
	asm("nop");
	
	// Then intialize the DAC interrupts
	*P_INT_MASK_CTRL1 = ~0x00000001;
	*P_DAC_CLK_CONF = 0x00000003;
	*P_DAC_MODE_CTRL1 = ~0x00000003;
}

void load_bg_music(const char *filename){
	
	FATFS fs0;
	FIL fil;
	FRESULT fr;
	UINT br;
	
	int file_size = 0;
	
	// Mount the USB drive with FatFS support
	f_mount(&fs0, "0:", 1);
	
	// Open background music MP3
	fr = f_open(&fil, filename, FA_READ);

	// Get size of background music MP3
	file_size = f_size(&fil);

	// Create a buffer for storing the background music MP3
	unsigned char *mp3ptr = (unsigned char *)LOAD_ADDRESS;
	
	// Read the background music MP3 into buffer
	fr = f_read(&fil, mp3ptr, file_size, &br);
	
	// Close file
	f_close(&fil);
	
	// Unmount
    f_mount(NULL, "0:", 0);
    
    // Address points to mp3 buffer
	Address = (unsigned short*)mp3ptr; 

	// Play mp3
	Play_MP3((unsigned int)Address,file_size/2);
	
	// Repeat
	Repeat_ON_MP3();
}

int get_dir_list_size(){
	
	int dir_count = 0;
	
	FATFS fs0; // FatFS mountpoint
    FRESULT res; // FatFs result variable
    DIR dir; // Directory object
    FILINFO fno; // File information object

    // Mount the file system
    res = f_mount(&fs0, "", 1);
    
    if (res != FR_OK) {
        printf("Failed to mount the file system. Error code: %d\n", res);
        while(1);
    }

    // Open the root directory
    res = f_opendir(&dir, "/apps");
    if (res != FR_OK) {
        printf("Failed to open the root directory. Error code: %d\n", res);
        while(1);
    }

    for (;;) {
    	// Read a directory item
        res = f_readdir(&dir, &fno);

		// Break on error or end of directory
        if (res != FR_OK || fno.fname[0] == 0) break;

		// Check if it's a directory
        if (fno.fattrib & AM_DIR) {
			dir_count++;
        }
    }

    // Close the directory
    f_closedir(&dir);

    // Unmount the file system
    f_mount(NULL, "", 0);

    return dir_count;
}

int load_directory_list(char **dir_buf){
	
	int dir_count = 0;
	
	FATFS fs0; // FatFS mountpoint
    FRESULT res; // FatFs result variable
    DIR dir; // Directory object
    FILINFO fno; // File information object

    // Mount the file system
    res = f_mount(&fs0, "", 1);
    
    if (res != FR_OK) {
        printf("Failed to mount the file system. Error code: %d\n", res);
        while(1);
    }

    // Open the root directory
    res = f_opendir(&dir, "/apps");
    if (res != FR_OK) {
        printf("Failed to open the root directory. Error code: %d\n", res);
        while(1);
    }

    for (;;) {
    	// Read a directory item
        res = f_readdir(&dir, &fno);

		// Break on error or end of directory
        if (res != FR_OK || fno.fname[0] == 0) break;

		// Check if it's a directory
        if (fno.fattrib & AM_DIR) {
			dir_buf[dir_count] = (char *)malloc(strlen(fno.fname) + 1);
			sprintf(dir_buf[dir_count], "%s", fno.fname);
			dir_count++;
        }
    }

    // Close the directory
    f_closedir(&dir);

    // Unmount the file system
    f_mount(NULL, "", 0);

    return 0;
}

static int show_selection(char **dir_buf, int dir_count, int index, int selection){
	int i = 0;

	if(dir_count >= 16) dir_count = 16;
	
	if(selection < 0) selection = 0;
	if(selection > 16) selection = 16;
	if(selection > dir_count - 1) selection = dir_count - 1;

	for(i=index;i<dir_count-index;i++){
		
		if(i == selection){
			tv_print((unsigned short *)FRAMEBUFFER_ADDRESS, 34, 8+i, "-->");
			tv_printcolor((unsigned short *)FRAMEBUFFER_ADDRESS, 37, 8+i, dir_buf[i], 0x7E0);
		}
		else{
			tv_print((unsigned short *)FRAMEBUFFER_ADDRESS, 34, 8+i, "   ");
			tv_print((unsigned short *)FRAMEBUFFER_ADDRESS, 37, 8+i, dir_buf[i]);
		}
	}
	
	return selection;
}
    
void execute_binary(char *dir_buf){

	int file_size = 0;
	
	unsigned int *ldrptr = (unsigned int *)LOAD_ADDRESS;
	void (*entry_start)(void) = (void *)ENTRY_ADDRESS;
	
	FATFS fs0; // FatFS mountpoint
	FIL fil;
    FRESULT res; // FatFs result variable
	UINT br;
	
    // Mount the file system
    res = f_mount(&fs0, "", 1);
    
    if (res != FR_OK) {
        printf("Failed to mount the file system. Error code: %d\n", res);
        while(1);
    }

    // Open the file
	char *filepath = (char *)malloc(strlen("/apps/")+strlen(dir_buf)+strlen("hyper.exe"));
	sprintf(filepath, "/apps/%s/hyper.exe", dir_buf);
	
    res = f_open(&fil, filepath, FA_READ);

	file_size = f_size(&fil);
	
	res = f_read(&fil, ldrptr, file_size, &br);

    // Close the directory
    f_close(&fil);

    // Unmount the file system
    f_mount(NULL, "", 0);
	
	entry_start();
}

static inline unsigned int asm_j_insn(unsigned int address, unsigned int link) {
    // low half: bits [14:0] of address, plus the p-bit at bit 15
    unsigned int insn_l = (address & 0x7FFFU)    // mask low 15 bits
                    | (1U << 15);            // set p-bit

    // high half: bits [28:16] of (address<<1), plus bits 31 and 27
    unsigned int insn_h = ((address << 1) & 0x1FFF0000U)  // mask bits [28:16]
                    | (1U << 31)                   // set bit 31
                    | (1U << 27);                  // set bit 27

    // combine halves and add the link field
    unsigned int assembled = (insn_h & 0xFFFF0000U)    // upper 16 bits
                       | (insn_l & 0x0000FFFFU);  // lower 16 bits
    assembled += link;

    return assembled;
}

int main(){

	// Initialize DAC interrupt handling
	//dac_Init();
	
	attach_isr(61, MP3_Service_Loop_ISR);
	    
	// Stupid Framebuffer
	unsigned short *fb = (unsigned short *)FRAMEBUFFER_ADDRESS;
	
	int dir_count = 0;
	int nExitCode = 0;
	int selection = 0;
	
	char *loading = "- NOW LOADING -";
	char *header1 = "--- HyperScan USB Loader ---";
	char *header2 = "--- ppcasm (ppcasm@gmail.com) ---";
	char *header3 = "Join the Discord: https://discord.gg/rHh2nW9sue";

	/************************************************************************/
	/*   TODO: add your code here                                           */
	/************************************************************************/
	
	/* Initalize Mattel HyperScan controller interface */
	hs_controller_init();
	
	// If start is held at boot of usbload then continue as normal
	hs_controller_read();
	if(controller[hs_controller_1].input.start){
		int i = 0;
		
		volatile unsigned int *src = (volatile unsigned int *)0x9F001000;
		volatile unsigned int *dst = (volatile unsigned int *)0xA00001FC;
		unsigned int n = (0xFF000 / 4);

		for(i = 0; i < n; i++) {
			dst[i] = src[i];
		}
		
		// You could place firmware patches here that would get applied before booting HyperScanOS
		// RESET PATCH EXAMPLE (resets when printing from coords takes place)
		//*(volatile unsigned int *)0xa000d2b0 = 0x84D88080;
		//*(volatile unsigned int *)0xa000d2b4 = 0x94FA9042;
		//*(volatile unsigned int *)0xa000d2b8 = 0x267c0000;
		//*(volatile unsigned int *)0xa000d2b0 = asm_j_insn(0xA0091000, 0);
		//*(volatile unsigned int *)0xa000d2b4 = 0x00000000;
		
		void (*entry_start)(void) = (void *)0xA0001000;
		entry_start();
	}
	
	/*
	Set TV output up with RGB565 color scheme and make set all framebuffers
	to stupid framebuffer address, tv_init will select the first framebuffer
	as default.
	*/
	tv_init(RESOLUTION_640_480, COLOR_RGB565, FRAMEBUFFER_ADDRESS, FRAMEBUFFER_ADDRESS, FRAMEBUFFER_ADDRESS);

	tv_print(fb, (((640/8)-strlen(header1))/2), 2, header1);
	tv_print(fb, (((640/8)-strlen(header2))/2), 3, header2);
	tv_print(fb, (((640/8)-strlen(header3))/2), 4, header3);

    menu_border(fb, (FRAMEBUFFER_WIDTH - 250) / 2, 20 + (FRAMEBUFFER_HEIGHT - 320) / 2, 250, 320, 1, 6, 0xFFFF); 

	dir_count = get_dir_list_size();

	char *dir_buf[dir_count];
	
	load_directory_list(dir_buf);
	
	show_selection(dir_buf, dir_count, 0, selection);

	while(1){
		
		// If right trigger is pressed, move menu select down
		hs_controller_read();
		if(controller[hs_controller_1].input.rt){
			selection++;
			selection = show_selection(dir_buf, dir_count, 0, selection);
		}
		
		// If left trigger is pressed, move menu select up
		hs_controller_read();
		if(controller[hs_controller_1].input.lt){
			selection--;
			selection = show_selection(dir_buf, dir_count, 0, selection);
		}
		
		// If start is pressed, load and execute Hyper.Exe
		hs_controller_read();
		if(controller[hs_controller_1].input.start){
			tv_print((unsigned short *)FRAMEBUFFER_ADDRESS, 20, 28, "                    ");
			tv_print((unsigned short *)FRAMEBUFFER_ADDRESS, 40-(strlen(loading)/2), 27, loading);
			tv_print((unsigned short *)FRAMEBUFFER_ADDRESS, 40-(strlen(dir_buf[selection])/2), 28, dir_buf[selection]);
			execute_binary(dir_buf[selection]);
		}
		
		hs_controller_read();
		if(controller[hs_controller_1].input.select){
			tv_print((unsigned short *)FRAMEBUFFER_ADDRESS, 40-(strlen("MP3 Mode")/2), 27, "MP3 Mode");

			int mp3_mode = 1;
			int count = 0;
			
			load_bg_music("/mp3/bg.mp3");

			while(mp3_mode){
				// Handle MP3 stream
				for(count=0;count<=100000;count++) MP3_Service_Loop();
	
				hs_controller_read();
				MP3_Service_Loop();
					
				if(controller[hs_controller_1].input.select) mp3_mode = 0;
	
			}
			tv_print((unsigned short *)FRAMEBUFFER_ADDRESS, 40-(strlen("MP3 Mode")/2), 27, "         ");
			Stop_MP3();
			for(count=0;count<=100000;count++);
		}
		
	}
	
	return nExitCode;
}

