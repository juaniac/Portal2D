#include "graphics_sub_engine.h"
#include "target_sub.h"
#include "portal_gun_sub.h"
#include <Bg_sub_startup.h>
#include <Bg_sub_ending.h>
#include <math.h>

void init_sub_bg3_for_gameplay();
void init_sub_bg2_for_gameplay();
float get_angle_rads(int x, int y);

//Setup the sub engine with the startup background
//which is displayed in mode 0, using background 0 (tile mode).
void init_sub_startup(){
	VRAM_C_CR = VRAM_ENABLE | VRAM_C_SUB_BG;
	REG_DISPCNT_SUB = MODE_0_2D | DISPLAY_BG0_ACTIVE;

	BGCTRL_SUB[0] = BG_COLOR_256 | BG_MAP_BASE(0) | BG_TILE_BASE(1) | BG_32x32;

	swiCopy(Bg_sub_startupPal, BG_PALETTE_SUB, Bg_sub_startupPalLen/2);
	swiCopy(Bg_sub_startupTiles, BG_TILE_RAM_SUB(1), Bg_sub_startupTilesLen/2);
	swiCopy(Bg_sub_startupMap, BG_MAP_RAM_SUB(0), Bg_sub_startupMapLen/2);
}

//Setup the main engine with the gameplay background
//which is displayed in mode 5, using background 2 and 3 (extended rotoscale mode).
void init_sub_for_gameplay(){
    VRAM_C_CR = VRAM_ENABLE | VRAM_C_SUB_BG;
	REG_DISPCNT_SUB = MODE_5_2D | DISPLAY_BG3_ACTIVE | DISPLAY_BG2_ACTIVE;

    init_sub_bg3_for_gameplay();
    init_sub_bg2_for_gameplay();
}

//Setup for background 3 in extended rotoscale mode with the gameplay background.
void init_sub_bg3_for_gameplay(){
    BGCTRL_SUB[3] = BG_BMP_BASE(0) | BgSize_B8_256x256;
    
    swiCopy(target_subPal, BG_PALETTE_SUB, target_subPalLen/2);
    swiCopy(target_subBitmap, BG_BMP_RAM_SUB(0), target_subBitmapLen/2);

    REG_BG3PA_SUB = (1<<8);
    REG_BG3PC_SUB = 0;
    REG_BG3PB_SUB = 0;
    REG_BG3PD_SUB = (1<<8);
}

#define PG_IMAGE_WIDTH 128
#define PG_MARGIN_X_PIX ((SCREEN_WIDTH - PG_IMAGE_WIDTH)/2)
#define PG_MARGIN_Y_PIX ((SCREEN_HEIGHT - PG_IMAGE_WIDTH)/2)
#define PG_CENTER_X (SCREEN_WIDTH/2)
#define PG_CENTER_Y (SCREEN_HEIGHT/2)

//Setup for background 2 in extended rotoscale mode with the gameplay background.
void init_sub_bg2_for_gameplay(){
    BGCTRL_SUB[2] = BG_BMP_BASE(3) | BgSize_B16_128x128;
    swiCopy(portal_gun_subBitmap, BG_BMP_RAM_SUB(3), portal_gun_subBitmapLen);

    sub_bg2_reset_angle();
}

//Given the screen coordinates x and y, rotate the background with the portal gun. 
float rotate_sub_bg2_for_gameplay(int x, int y){
    float angle_rads = get_angle_rads(x, y);
    
    float r = sqrt(PG_CENTER_X*PG_CENTER_X + PG_CENTER_Y*PG_CENTER_Y);
    float alpha = atan((float)PG_CENTER_X/(float)PG_CENTER_Y) + angle_rads;

    REG_BG2PA_SUB = cos(angle_rads) * 256;
    REG_BG2PB_SUB = sin(angle_rads) * 256;
    REG_BG2PC_SUB = -sin(angle_rads) * 256;
    REG_BG2PD_SUB = cos(angle_rads) * 256; 

    REG_BG2X_SUB = (PG_CENTER_X - r*sin(alpha) - PG_MARGIN_X_PIX) * 256;
    REG_BG2Y_SUB = (PG_CENTER_Y - r*cos(alpha) - PG_MARGIN_Y_PIX) * 256;

    return angle_rads; 
}

//Given the screen coordinates x and y, 
//return the angle between that point and horizontal line in radiants 
float get_angle_rads(int x, int y){
    //translate x and y with respect to the center of the portal gun
    x = x - PG_CENTER_X;
    y = y - PG_CENTER_Y; 

    //scale x and y to be between -1 and 1.
    float r = sqrt(x*x + y*y);
    float x_f = (float)x / r;
    float y_f = (float)y / r;

    //get the angle of the point (x,y) in radians from top going to the right
    return ((y_f < 0) ? 2*M_PI - acos(x_f) : acos(x_f));
}

//Resets the background rotational angle to 0 by changing the affine transformation matrix
void sub_bg2_reset_angle(){
    REG_BG2PA_SUB = (1<<8);
    REG_BG2PB_SUB = 0;
    REG_BG2PC_SUB = 0;
    REG_BG2PD_SUB = (1<<8); 

    REG_BG2X_SUB = -(PG_MARGIN_X_PIX<<8);
    REG_BG2Y_SUB = -(PG_MARGIN_Y_PIX<<8);
}

//Setup the sub engine with the ending background
//which is displayed in mode 0, using background 0 (tile mode).
void init_sub_ending(){
	VRAM_C_CR = VRAM_ENABLE | VRAM_C_SUB_BG;
	REG_DISPCNT_SUB = MODE_0_2D | DISPLAY_BG0_ACTIVE;

	BGCTRL_SUB[0] = BG_COLOR_256 | BG_MAP_BASE(0) | BG_TILE_BASE(1) | BG_32x32;

	swiCopy(Bg_sub_endingPal, BG_PALETTE_SUB, Bg_sub_endingPalLen/2);
	swiCopy(Bg_sub_endingTiles, BG_TILE_RAM_SUB(1), Bg_sub_endingTilesLen/2);
	swiCopy(Bg_sub_endingMap, BG_MAP_RAM_SUB(0), Bg_sub_endingMapLen/2);
}