/*

This is part of the Mattel HyperScan SDK by ppcasm (ppcasm@gmail.com)

This is the basic example of how to read from CD.

To run this example, create a file named "test.txt" with some text in
it, and save it on the root of the CD. 

Note: Make sure the text isn't over 128 characters long for this simple example.

*/

#include <stdio.h>
#include "tv/tv.h"
#include "hyperscan/hyperscan.h"
#include "uart/uart.h"

// Stupid framebuffer for now
unsigned short *fb = (unsigned short *) 0xA0400000;

int main(){
	
	int nExitCode = 0;
	/************************************************************************/
	/*   TODO: add your code here                                           */
	/************************************************************************/
	
	/*
	 Set TV output up with RGB565 color scheme and make set all framebuffers
	 to stupid framebuffer address, TV_Init will select the first framebuffer
	 as default.
	*/
	tv_init(RESOLUTION_640_480, COLOR_RGB565, 0xA0400000, 0xA0400000, 0xA0400000);

	while(1){
				
		char buf[128];
		
		// Initialize CDRom drive and appropriate structures/globals
		iso_init();
		
		// Open file, and get descriptor, -1 if error
		int filedes = iso_open("test.txt");
		if (filedes == -1){
			tv_print(fb, 1, 1, "There was an error opening the file");
			while(1);
		}
		
		// Get size by seeking to end
		int size = iso_lseek(filedes, 0, SEEK_END);
		
		if (size < 0){
			tv_print(fb, 1, 1, "Error while seeking end");
			while(1);
		}
		
		if (iso_lseek(filedes, 0, SEEK_SET) < 0){
			tv_print(fb, 1, 1, "Error while seeking to start");
			while(1);
		}
		

		// Clamp size if file is larger than size of buf
		if (size > sizeof(buf)){
			size = sizeof(buf); 
		}
		
		// Read data from CD into buffer, -1 if error
		int read_ret = iso_read(filedes, buf, size);
		if (read_ret == -1){
			tv_print(fb, 1, 1, "There was an error opening the file");
			while(1);
		}
		
		// Now close the file, freeing the file handle		
		iso_close(filedes);
		
		while(1) {
			tv_print(fb, 1, 1, buf); 
		}		
	}
		
	return nExitCode;
}
