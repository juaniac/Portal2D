#include <stdio.h>
#include <Bg_main_gameplay.h>
#include <Bg_main_startup.h>
#include <Bg_main_ending.h>
#include "graphics_main_engine.h"
#include "sprite_management.h"
#include "map.h"

//Setup the main engine with the startup background
//which is displayed in mode 0, using background 0 (tile mode).
void init_main_startup(){
	VRAM_A_CR = VRAM_ENABLE | VRAM_A_MAIN_BG;
	REG_DISPCNT = MODE_0_2D | DISPLAY_BG0_ACTIVE;

	BGCTRL[0] = BG_COLOR_256 | BG_MAP_BASE(0) | BG_TILE_BASE(1) | BG_32x32;

	swiCopy(Bg_main_startupPal, BG_PALETTE, Bg_main_startupPalLen/2);
	swiCopy(Bg_main_startupTiles, BG_TILE_RAM(1), Bg_main_startupTilesLen/2);
	swiCopy(Bg_main_startupMap, BG_MAP_RAM(0), Bg_main_startupMapLen/2);
}

//Setup the main engine with the gameplay background
//which is displayed in mode 0, using background 2 and 3 (tile mode).
//level_id serves to pick which level map we will load into the tile VRAM.
void init_main_for_gameplay(int level_id){
    VRAM_A_CR = VRAM_ENABLE | VRAM_A_MAIN_BG;
	REG_DISPCNT = MODE_0_2D | DISPLAY_BG2_ACTIVE | DISPLAY_BG3_ACTIVE;

	//Shared palette and Tiles for main gameplay display
	BGCTRL[2] = BG_COLOR_256 | BG_MAP_BASE(MAP_RAM_B2_OFFSET) | BG_TILE_BASE(1) | BG_32x32;
	BGCTRL[3] = BG_COLOR_256 | BG_MAP_BASE(MAP_RAM_B3_OFFSET) | BG_TILE_BASE(1) | BG_32x32;

	swiCopy(Bg_main_gameplayPal, BG_PALETTE, Bg_main_gameplayPalLen/2);
	swiCopy(Bg_main_gameplayTiles, BG_TILE_RAM(1), Bg_main_gameplayTilesLen/2);
	
	loadBackground3Map();
	if(level_id == 1){
		loadLevel1();
	}else if(level_id == 2){
		loadLevel2();
	}if(level_id == 3){
		loadLevel3();
	}if(level_id == 4){
		loadLevel4();	
	}
}

//Setup the main engine with the startup background
//which is displayed in mode 0, using background 0 (tile mode).
void init_main_ending(){
	VRAM_A_CR = VRAM_ENABLE | VRAM_A_MAIN_BG;
	REG_DISPCNT = MODE_0_2D | DISPLAY_BG0_ACTIVE;

	BGCTRL[0] = BG_COLOR_256 | BG_MAP_BASE(0) | BG_TILE_BASE(1) | BG_32x32;

	swiCopy(Bg_main_endingPal, BG_PALETTE, Bg_main_endingPalLen/2);
	swiCopy(Bg_main_endingTiles, BG_TILE_RAM(1), Bg_main_endingTilesLen/2);
	swiCopy(Bg_main_endingMap, BG_MAP_RAM(0), Bg_main_endingMapLen/2);
}