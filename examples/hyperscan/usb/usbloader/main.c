/*

This is part of the Mattel HyperScan SDK by ppcasm (ppcasm@gmail.com)
 
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "tv/tv.h"
#include "uart/uart.h"
#include "irq/interrupts.h"
#include "hyperscan/fatfs/ff.h"
#include "hyperscan/hyperscan.h"

#include "score7_registers.h"
#include "score7_constants.h"
#include "hyperscan/hs_controller/hs_controller.h"

#define FRAMEBUFFER_ADDRESS 0xA0400000
#define FRAMEBUFFER_WIDTH   640
#define FRAMEBUFFER_HEIGHT  480

#define LOAD_ADDRESS 0xA00901FC
#define ENTRY_ADDRESS 0xA0091000

static FATFS fs0; //FatFS mountpoint
static FIL callback_fil;
static char filename_buf[128];

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

int get_dir_list_size(){
	
	FILINFO fno; // File information object
	DIR dir; // Directory object
	FRESULT res; // FatFs result variable
	
	int dir_count = 0;
	
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

	FILINFO fno; // File information object
	DIR dir; // Directory object
	FRESULT res; // FatFs result variable
	
	int dir_count = 0;
	
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

	FRESULT res; // FatFs result variable
	UINT br;
	FIL fil;
	
	int file_size = 0;
	
	unsigned int *ldrptr = (unsigned int *)LOAD_ADDRESS;
	void (*entry_start)(void) = (void *)ENTRY_ADDRESS;
	
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

int iso_init_callback() {
	asm("la r18, _gp");
	
	HS_LEDS(0x01);
	printf("iso_init... Go fuck yourself\n");
	return 1;
}

int iso_open_callback(char *filename) {
	asm("la r28, _gp");
	
	FRESULT res; // FatFs result variable

	HS_LEDS(0x02);
	printf("iso_open: %s\n", filename);
	
    // Mount the file system
    res = f_mount(&fs0, "", 1);
    
    if (res != FR_OK) {
        printf("Failed to mount the file system. Error code: %d\n", res);
        while(1);
    }

    // Open the file
	char *filepath = (char *)malloc(128);
	sprintf(filepath, "%s", filename);
	
    res = f_open(&callback_fil, filepath, FA_READ);
    
    strncpy(filename_buf, filename, 128);
    
    return 1;
}

void iso_read_callback(int filedes, void *buffer, unsigned int size) {
	asm("la r28, _gp");

	FRESULT res; // FatFs result variable
	UINT br;
	
	HS_LEDS(0x04);

	printf("iso_read: filedes(%x) | buffer{%p) | size(%x)\n", filedes, buffer, size);
	
	res = f_read(&callback_fil, buffer, size, &br);

	if(strcmp(filename_buf, "DAT\\IWL.EXE") == 0){
		printf("Patching stack pointer\n");
		*(unsigned int *)0xa00a409c = 0x00000000;
		*(unsigned int *)0xa00a40a0 = 0x00000000;
		*(unsigned int *)0xa00a40a4 = asm_j_insn(0xA005E000, 1);
	}
	
    // Close the directory
    f_close(&callback_fil);
}

int iso_lseek_callback(int fd, int offset, int whence) {
	asm("la r28, _gp");
	HS_LEDS(0x08);
	
	while(1) { printf("iso_lseek");}
	
	return 0;
}

void iso_close_callback(int filedes) {
	asm("la r28, _gp");
	HS_LEDS(0xF0);
	
	printf("iso_close()\n");

    // Unmount the file system
    f_mount(NULL, "", 0);
}

int main(){

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
	
	if(controller[hs_controller_1].input.ls && controller[hs_controller_1].input.rs){
		printf("Dumping RAM...\n");
		
		UINT bw;
		FIL fil;
		FRESULT res; // FatFs result variable

		unsigned char *ram_ptr = (unsigned char *)0xA0000000;
		unsigned int chunk_size = (4 * 1024);
		unsigned int total_sectors = (16 * (1024 * 1024)) / chunk_size;
		unsigned char save_buffer[chunk_size];
		
		int count = 0;
		
	    // Mount the file system
		res = f_mount(&fs0, "", 1);
    
		if (res != FR_OK) {
			printf("Failed to mount the file system. Error code: %d\n", res);
			while(1);
		}
		
		res = f_open(&fil, "ramdump.bin", FA_WRITE | FA_CREATE_ALWAYS);
		if (res != FR_OK) {
			printf("Failed to open the file. Error code: %d\n", res);
			while(1);    
    	}
    	
    	for(count=0;count<total_sectors;count++) {
    		
    		memcpy((void *)save_buffer, (const void *)ram_ptr, chunk_size);
    		
    		res = f_write(&fil, save_buffer, chunk_size, &bw);
    		
    		if(res != FR_OK || bw < chunk_size) {
    			printf("Failed to write the file. Error code: %d\n", res);
    			f_close(&fil);
    			while(1); 
    		}
    		
    		ram_ptr += chunk_size;
    		printf("Sector: %d - Addr: %p\n", count, ram_ptr);

			res = f_sync(&fil);
    		if(res != FR_OK) {
				printf("Failed to commit. Error code: %d\n", res);
			f_close(&fil);
			while(1);
    		}
    	}
    	
		printf("Done!\n");
	}
	
	if(controller[hs_controller_1].input.start){
		int i = 0;
		
		volatile unsigned int *src = (volatile unsigned int *)0x9F001000;
		volatile unsigned int *dst = (volatile unsigned int *)0x800001FC;
		unsigned int n = (0xFF000 / 4);

		// invalidate D-Cache
		asm("cache 0x18, [r15, 0]");
		asm("nop");
		asm("nop!");
		asm("nop!");
		asm("nop!");
		asm("nop!");
		asm("nop");
		
		//	invalidate I-Cache
		asm("cache 0x10, [r15, 0]");
		asm("nop");
		asm("nop!");
		asm("nop!");
		asm("nop!");
		asm("nop!");
		asm("nop");

		// Drain write buffer
		asm("cache 0x1A, [r15, 0]");
		asm("nop");
		asm("nop!");
		asm("nop!");
		asm("nop!");
		asm("nop!");
		asm("nop");
		
		for(i = 0; i < n; i++) {
			dst[i] = src[i];
		}
		
		// invalidate D-Cache
		asm("cache 0x18, [r15, 0]");
		asm("nop");
		asm("nop!");
		asm("nop!");
		asm("nop!");
		asm("nop!");
		asm("nop");
		
		//	invalidate I-Cache
		asm("cache 0x10, [r15, 0]");
		asm("nop");
		asm("nop!");
		asm("nop!");
		asm("nop!");
		asm("nop!");
		asm("nop");

		// Drain write buffer
		asm("cache 0x1A, [r15, 0]");
		asm("nop");
		asm("nop!");
		asm("nop!");
		asm("nop!");
		asm("nop!");
		asm("nop");
		
		// You could place firmware patches here that would get applied before booting HyperScanOS

		// Patch iso_init callback
		//*(volatile unsigned int *)0x80000890 = 0x00000000; 
		//*(volatile unsigned int *)0x80000894 = 0x00000000;
		//*(volatile unsigned int *)0x80000898 = asm_j_insn((unsigned int)iso_init_callback, 0);

		// Patch iso_open callback
		//*(volatile unsigned int *)0x8000089C = 0x00000000;
		//*(volatile unsigned int *)0x800008A0 = 0x00000000;
		//*(volatile unsigned int *)0x800008A4 = asm_j_insn((unsigned int)iso_open_callback, 0);

		// Patch iso_read callback
		//*(volatile unsigned int *)0x800008A8 = 0x00000000;
		//*(volatile unsigned int *)0x800008AC = 0x00000000;
		//*(volatile unsigned int *)0x800008B0 = asm_j_insn((unsigned int)iso_read_callback, 0);

		// Patch iso_lseek callback
		//*(volatile unsigned int *)0x800008B4 = 0x00000000;
		//*(volatile unsigned int *)0x800008B8 = 0x00000000;
		//*(volatile unsigned int *)0x800008BC = asm_j_insn((unsigned int)iso_lseek_callback, 0);

		// Patch iso_close callback
		//*(volatile unsigned int *)0x800008C0 = 0x00000000;
		//*(volatile unsigned int *)0x800008C4 = 0x00000000;
		//*(volatile unsigned int *)0x800008C8 = asm_j_insn((unsigned int)iso_close_callback, 0);

		// Patch checksum
		//*(volatile unsigned int *)0xA000F740 = 0x84B88018;
		
		// invalidate D-Cache
		asm("cache 0x18, [r15, 0]");
		asm("nop");
		asm("nop!");
		asm("nop!");
		asm("nop!");
		asm("nop!");
		asm("nop");
		
		//	invalidate I-Cache
		asm("cache 0x10, [r15, 0]");
		asm("nop");
		asm("nop!");
		asm("nop!");
		asm("nop!");
		asm("nop!");
		asm("nop");

		// Drain write buffer
		asm("cache 0x1A, [r15, 0]");
		asm("nop");
		asm("nop!");
		asm("nop!");
		asm("nop!");
		asm("nop!");
		asm("nop");
		
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


	// patch timer int handler
	volatile unsigned int *timerpatch = (volatile unsigned int *)0x8000c62c;
	int z = 0;
	for(z=0;z<=0x80;z++){
		timerpatch[z] = 0x00000000;
	}
	//*(volatile unsigned int *)0x8000C62C = 0x10804084;
	//*(volatile unsigned int *)0x8000C630 = 0x00002300;
	//*(volatile unsigned int *)0x8000C634 = 0x0a230a22;
	//*(volatile unsigned int *)0x8000C638 = 0x0000340f;
	
	// Patch iso_init callback
	*(volatile unsigned int *)0x80000890 = 0x00000000; 
	*(volatile unsigned int *)0x80000894 = 0x00000000;
	*(volatile unsigned int *)0x80000898 = asm_j_insn((unsigned int)iso_init_callback, 0);
		
	// Patch iso_open callback
	*(volatile unsigned int *)0x8000089C = 0x00000000;
	*(volatile unsigned int *)0x800008A0 = 0x00000000;
	*(volatile unsigned int *)0x800008A4 = asm_j_insn((unsigned int)iso_open_callback, 0);

	// Patch iso_read callback
	*(volatile unsigned int *)0x800008A8 = 0x00000000;
	*(volatile unsigned int *)0x800008AC = 0x00000000;
	*(volatile unsigned int *)0x800008B0 = asm_j_insn((unsigned int)iso_read_callback, 0);

	// Patch iso_lseek callback
	*(volatile unsigned int *)0x800008B4 = 0x00000000;
	*(volatile unsigned int *)0x800008B8 = 0x00000000;
	*(volatile unsigned int *)0x800008BC = asm_j_insn((unsigned int)iso_lseek_callback, 0);

	// Patch iso_close callback
	*(volatile unsigned int *)0x800008C0 = 0x00000000;
	*(volatile unsigned int *)0x800008C4 = 0x00000000;
	*(volatile unsigned int *)0x800008C8 = asm_j_insn((unsigned int)iso_close_callback, 0);
		
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
	}
	
	return nExitCode;
}

