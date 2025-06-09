#include "score7_registers.h"
#include "score7_constants.h"

//
//  ppu_init - Initialize PPU
//
void ppu_init(void) {
}

//
//  ppu_getspritepaletteaddr - Get the address of the sprite palette
//  - added by Hyperscandev and merged on 6/9/25
//
//  Args - bank
//  Return - Address of palette
UV32* ppu_getspritepaletteaddr(unsigned int bank) {
	return P_SPRITE_ATTRIBUTE_BASE + (8 * bank);
}

//
//  ppu_getspritexpos - Get the sprite X position
//  - added by Hyperscandev and merged on 6/9/25
//
//  Args - id
//  Return - Sprite X position
unsigned int ppu_getspritexpos(unsigned int id) {
	return *((UV32*)(P_SPRITE_X_BASE + (8 * id)));
}

//
//  ppu_getspritexpos - Get the sprite Y position
//  - added by Hyperscandev and merged on 6/9/25
//
//  Args - id
//  Return - Sprite Y position
unsigned int ppu_getspriteypos(unsigned int id) {
	return *((UV32*)(P_SPRITE_X_BASE + (8 * id)));
}

