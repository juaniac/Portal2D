#include "sprite_management.h"
#include "graphics_main_engine.h"
#include "graphics_sub_engine.h"
#include <stdio.h>
#include "all_sprites.h"
#include "sound_management.h"
#include "utils.h"
#include <math.h>
#include "irq_management.h"

#define PLAYER_START_SPEED_X 0
#define PLAYER_START_SPEED_Y 0
#define PRESS_SPEED 200
#define MAX_RUNNING_SPEED 300
#define JUMP_SPEED -50
#define GRAVITY_CONST 9.81*8.0*2.0 //pixel / sec^2, 1m = 8px * 2 = 2 tiles
#define DRAG_CONST 5
#define DELTA_TIME_CONST 0.016
#define SLOW_DOWN_TIME_BY 20
#define AIR_SLOW_DOWN 4
#define DRAG_AIR_SLOW_DOWN 3
double delta_time = 0.016;

#define UPDATE_SPRITES 10
int update_sprite_counter = 0;
#define CHELL_SPRITE_ID 0
#define GUN_SPRITE_ID 1
#define BLUE_PORTAL_BASE_SPRITE_ID 2
#define ORANGE_PORTAL_BASE_SPRITE_ID 5
#define PORJECTILE_BASE_SPRITE_ID 8

#define NB_CHELL_SPRITES 7
u16* chell_sprites[NB_CHELL_SPRITES];
typedef enum {IDLE, RIGHT1, RIGHT2, LEFT1, LEFT2, JUMP, GUN} Chell_Sprite_states;
Chell_Sprite_states chell_sprite_state;

Chell_Sprite_states chell_walk_cycle[] = {IDLE, RIGHT1, RIGHT2, RIGHT1, IDLE, LEFT1, LEFT2, LEFT1};
size_t walk_cycle_state;

typedef struct {
	double x;
	double y;
} Speed;

typedef enum {Orange, Blue}Portal_color;
typedef struct{
	Point pos;
	Speed speed;
	Portal_color color;
}Portal_Projectile;
#define MAX_NUMBER_PROJECTILES 2
size_t nb_projectile_active;
Portal_Projectile* portal_projectile_list[MAX_NUMBER_PROJECTILES];
#define PORTAL_DELTA 250

typedef struct{
	bool active;
	size_t cycle_state;
	TilePoint topLeft;
	Direction dir;
}Portal;
Portal blue_portal;
Portal orange_portal;

//WARNING CLOCK WISE RANDIENT!!
float portal_gun_angle_rand;

#define NB_PORTAL_SPRITES 5
u16* orange_portal_sprites[NB_PORTAL_SPRITES];
u16* blue_portal_sprites[NB_PORTAL_SPRITES];

typedef enum {PROJECTILE, STATE1, STATE2, STATE3, STATE4}Portal_Sprite_states;
Portal_Sprite_states portal_cycle[] = {STATE1, STATE2, STATE3, STATE4};

#define PORTAL_SIZE_PIXELS 3*8

Point player_pos;
Speed player_speed;

Direction player_dir_x;
Direction player_dir_y;

bool has_gone_through_a_portal = false;

//Allocate and configure the character and the portals sprites
void configureSprites() {
	//Set up memory bank to work in sprite mode (offset since we are using VRAM A for backgrounds and Tiles)
	VRAM_B_CR = VRAM_ENABLE | VRAM_B_MAIN_SPRITE_0x06400000;

	//Initialize sprite manager and the engine
	oamInit(&oamMain, SpriteMapping_1D_32, false);

	//Allocate space for the data and place the Tiles in the VRAM
	int i;
	for(i = 0; i < NB_CHELL_SPRITES; i ++){
		chell_sprites[i] = oamAllocateGfx(&oamMain, SpriteSize_16x16, SpriteColorFormat_256Color);
		swiCopy(all_spritesTiles + 64 * i, chell_sprites[i], 256/2);
	}
	for(i = 0; i < NB_PORTAL_SPRITES; i ++){
		
		orange_portal_sprites[i] = oamAllocateGfx(&oamMain, SpriteSize_8x8, SpriteColorFormat_256Color);
		swiCopy(all_spritesTiles + 64 * (NB_CHELL_SPRITES) + i*32, orange_portal_sprites[i], 64/2);

		blue_portal_sprites[i] = oamAllocateGfx(&oamMain, SpriteSize_8x8, SpriteColorFormat_256Color);
		swiCopy(all_spritesTiles + 64 * (NB_CHELL_SPRITES) + i*32 + 16, blue_portal_sprites[i], 64/2);
	}

	//Copy the shared palette
	swiCopy(all_spritesPal, SPRITE_PALETTE, all_spritesPalLen/2);
	
}

//Checks if the player is near a block, useful for checking for doors or buttons
bool isPlayerNear(uint16 tiletype){
	int x, y;
	for(y = (int)(player_pos.y)-1; y < PLAYER_SIZE * TILE_SIZE + (int)(player_pos.y) ; y += TILE_SIZE){
		for(x = (int)(player_pos.x)-1; x < PLAYER_SIZE * TILE_SIZE + (int)(player_pos.x) ; x += TILE_SIZE){
			if(getTile(max((int)(player_pos.x), x)/TILE_SIZE, max((int)(player_pos.y), y)/TILE_SIZE) == tiletype){
				return true;
			}
		}
	}
	return false;
}

//Delete a projectile
void deleteProjectile(Portal_Projectile* proj, int id){
	oamClear(&oamMain, PORJECTILE_BASE_SPRITE_ID + id, 1);
	free(proj);
	portal_projectile_list[id] = NULL;
	nb_projectile_active -= 1;
}

//Initialise the player character (can be used as a reset)
void init_player() {
	//Sets the initial player variables
	player_pos.x = curMap.startingPoint.x * TILE_SIZE;
	player_pos.y = curMap.startingPoint.y * TILE_SIZE;
	player_speed.x = PLAYER_START_SPEED_X;
	player_speed.y = PLAYER_START_SPEED_Y;
	player_dir_x = Right;
	player_dir_y = None;	
	
	//Clears sprite data 
	oamClear(&oamMain, CHELL_SPRITE_ID, 1);
	oamClear(&oamMain, GUN_SPRITE_ID, 1);
	chell_sprite_state = IDLE; 
	walk_cycle_state = 0;
	update_sprite_counter = 0;

	//Give the correct angle to the portal gun
	portal_gun_angle_rand = 0;
	sub_bg2_reset_angle();
	
	//Remove any still standing projectiles
	nb_projectile_active = 0;
	int i;
	for(i = 0; i < MAX_NUMBER_PROJECTILES; i++){
		Portal_Projectile* proj = portal_projectile_list[i];
		if(proj != NULL){
			deleteProjectile(proj, i);
		}
	}

	//Deactivates both portals
	orange_portal.active = false;
	orange_portal.cycle_state = 0;
	oamClear(&oamMain, ORANGE_PORTAL_BASE_SPRITE_ID, 3);

	blue_portal.active = false;
	blue_portal.cycle_state = 0;
	oamClear(&oamMain, BLUE_PORTAL_BASE_SPRITE_ID, 3);

	//Disables the Timer if it was on
	disable_timer0();
}

//Returns true if player at (x,y) is hitting a wall (or forbidden tile) in Direction dir
bool isPlayerHittingWall(int x, int y, Direction dir) {
	bool isThirdTileOK = true;

	int tile_x, tile_y, tile_x_left, tile_x_right, tile_y_up, tile_y_down;

	if ((dir == Left) | (dir == Right)) {
		if ((x % TILE_SIZE) != 0) return false; //Player not horizontally tile-aligned in horizontal move => can move freely

		tile_y_up = (y / TILE_SIZE);
		tile_y_down = tile_y_up + 1;
		tile_x = (dir == Left) ? x / TILE_SIZE - 1 : x / TILE_SIZE + PLAYER_SIZE; // Left or right
		enum Tile tile_up = getTile(tile_x, tile_y_up);
		enum Tile tile_down = getTile(tile_x, tile_y_down);

		if ((y % TILE_SIZE) != 0) { //Player not tile-aligned, need to check third tile
			enum Tile third_tile = getTile(tile_x, tile_y_down + 1);
			isThirdTileOK = !((third_tile == Wall) || (third_tile == Forbidden));
		}
		return are2TilesHard(tile_up, tile_down) || !isThirdTileOK;
	} else { //dir = down or up
		if ((y % TILE_SIZE) != 0) return false; //Player not vertically tile-aligned in vertical move => can move freely

		tile_x_left = x / TILE_SIZE;
		tile_x_right = tile_x_left + 1;
		tile_y = (dir == Up) ? y / TILE_SIZE - 1 : (y / TILE_SIZE) + PLAYER_SIZE;
		enum Tile tile_left = getTile(tile_x_left, tile_y);
		enum Tile tile_right = getTile(tile_x_right, tile_y);

		if ((x % TILE_SIZE) != 0) { //Player not tile-aligned, need to check third tile
			enum Tile third_tile = getTile(tile_x_right + 1, tile_y);
			isThirdTileOK = !((third_tile == Wall) || (third_tile == Forbidden));
		}
		return are2TilesHard(tile_left, tile_right) || !isThirdTileOK;
	}
}

//Returns the segment that defines the portal area
Segment getPortalSegment(Portal portal){
	Segment res;
	res.start.x = portal.topLeft.x * TILE_SIZE + ((portal.dir == Right) ? TILE_SIZE - 1 : 0);
	res.start.y = portal.topLeft.y * TILE_SIZE + ((portal.dir == Down) ? TILE_SIZE - 1 : 0);
	res.end.x = res.start.x + ((portal.dir == Down || portal.dir == Up) ? PORTAL_SIZE_PIXELS - 1 : 0);
	res.end.y = res.start.y + ((portal.dir == Left || portal.dir == Right) ? PORTAL_SIZE_PIXELS - 1 : 0);
	return res;
}

//Changes the velocity and position vectors of the player depending on what portal they just
//entered and returns the point at which they come out of the end portal.
Point teleportPlayer(Portal fromPortal, Portal toPortal, double* delta_x, double* delta_y){
	double temp;
	has_gone_through_a_portal = true;

	//Portals in the same direction in their x axis 
	//means we must mirror in the x direction
	if((fromPortal.dir == Right && toPortal.dir == Right) || 
		(fromPortal.dir == Left && toPortal.dir == Left)){
		player_speed.x = -1*player_speed.x;
		player_speed.y = player_speed.y;
		player_dir_x = (player_dir_x == Left) ? Right : Left;
		(*delta_x) = -1*(*delta_x);
		(*delta_y) = (*delta_y);
	}//Portals in the same direction in their y axis,
	//means that we must mirror in the y direction
	else if((fromPortal.dir == Up && toPortal.dir == Up) || 
		(fromPortal.dir == Down && toPortal.dir == Down)){
		player_speed.x = player_speed.x;
		player_speed.y = -1*player_speed.y;
		(*delta_x) = (*delta_x);
		(*delta_y) = -1*(*delta_y);
	}//Portals with a difference of 90 degrees on their right 
	//means that we must also turn 90 their vectors.
	else if((fromPortal.dir == Up && toPortal.dir == Right) || 
		(fromPortal.dir == Down && toPortal.dir == Left) || 
		(fromPortal.dir == Right && toPortal.dir == Down) ||
		(fromPortal.dir == Left && toPortal.dir == Up)){
		temp = player_speed.x;
		player_speed.x = player_speed.y;
		player_speed.y = -1*temp;
		temp = (*delta_x);
		(*delta_x) = (*delta_y);
		(*delta_y) = -1*temp;
	}//Portals with a difference of 270 degrees on their right 
	//means that we must also turn 270 their vectors.
	else if((fromPortal.dir == Right && toPortal.dir == Up)||
		(fromPortal.dir == Left && toPortal.dir == Down)  || 
		(fromPortal.dir == Down && toPortal.dir == Right) || 
		(fromPortal.dir == Up && toPortal.dir == Left)){
		temp = player_speed.x;
		player_speed.x = -1*player_speed.y;
		player_speed.y = temp;
		temp = (*delta_x);
		(*delta_x) = -1*(*delta_y);
		(*delta_y) = temp;
	}

	//We compute the destination of the Player
	Point endPoint = {toPortal.topLeft.x * TILE_SIZE
						+ ((toPortal.dir == Down || toPortal.dir == Up) ? (PORTAL_SIZE_PIXELS - PLAYER_SIZE_PX)/2 : 0)
						+ ((toPortal.dir == Left) ? 1 : ((toPortal.dir == Right) ? -(TILE_SIZE + 1)  : 0)),
					  toPortal.topLeft.y * TILE_SIZE
					  	+ ((toPortal.dir == Right || toPortal.dir == Left) ? (PORTAL_SIZE_PIXELS - PLAYER_SIZE_PX)/2 : 0)
					  	+ ((toPortal.dir == Up) ? 1 : ((toPortal.dir == Down) ? -(TILE_SIZE + 1) : 0))};
	return endPoint;
}

//Returns farthest point where a player object of size tile_size_x by tile_size_y at (x, y) 
//can go before a collision when moving of (delta_x, delta_y) knowning it keeps its momentum. 
Point playerCollision(double x, double y, double delta_x, double delta_y, int tile_size_x, int tile_size_y) {

	//Example player 2 tiles x 2 tiles moving with dx > 0 or dy < 0, 5 segments to check, center segment on top right corner
	//	  /	  /    /
	//   /___/____/
	//  |    |    | /
	//	|____|____|/
	//	|	 |	  | /
	//  |____|____|/
	//
	//Which path to check depends on player direction (See drawing for details)
	bool dxNeg = delta_x < 0;
	bool dxPos = delta_x > 0;
	bool dyNeg = delta_y < 0;
	bool dyPos = delta_y > 0;

	//endPos a priori if no collision on path
	double maxDelta_x = delta_x;
	double maxDelta_y = delta_y;
	double speed_x = player_speed.x;
	double speed_y = player_speed.y;

	double portalMaxDelta_x = delta_x;
	double portalMaxDelta_y = delta_y;
	bool colisionWithOrange = false;
	bool colisionWithBlue = false;
	Segment orangeSegment = getPortalSegment(orange_portal);
	Segment blueSegment = getPortalSegment(blue_portal);


	size_t i;
	//There is movement in the x axis
	if(dxPos ^ dxNeg){
		double border_x = (dxNeg || tile_size_x == 0) ? x : (tile_size_x * TILE_SIZE + x -1) ;

		//Going through all points that may collide with a wall in the x axis
		double border_y;
		for(border_y = y-1; border_y < tile_size_y * TILE_SIZE + y ; border_y += TILE_SIZE){
			//Current point segment points
			Point start = {border_x , max(y, border_y)};
			Point end = {border_x + delta_x, max(y, border_y) + delta_y};

			//Check intersection of segment for all rectangles of map
			for(i = 0; i < curMap.nbMapWalls; i++) {
				Point newEnd = colisionResolvePlayer(start, end, curMap.wallList[i].rect);
				if (newEnd.x != -1 && (int)(newEnd.x) != (int)(end.x)) { //If collision on the x axis
					double intersecDelta_x = newEnd.x - start.x;
					maxDelta_x = doubleAbs(maxDelta_x) < doubleAbs(intersecDelta_x) ? maxDelta_x : intersecDelta_x;
					speed_x = 0;
				}
			}	

			//Check intersection of segments for portals
			if(orange_portal.active){
				Point newEnd = colisionResolvePlayerPortal(start, end, orangeSegment);
				if (newEnd.x != -1){ //If colision
					double intersecDelta_x = newEnd.x - start.x;
					double intersecDelta_y = newEnd.y - start.y;
					if(doubleAbs(intersecDelta_x) <= doubleAbs(portalMaxDelta_x) ||
					   doubleAbs(intersecDelta_y) <= doubleAbs(portalMaxDelta_y)){
						colisionWithOrange = true;
						colisionWithBlue = false;
						portalMaxDelta_x = intersecDelta_x;
						portalMaxDelta_y = intersecDelta_y;
					}
				}
			}
			if(blue_portal.active){
				Point newEnd = colisionResolvePlayerPortal(start, end, blueSegment);
				if (newEnd.x != -1){ //If colision
					double intersecDelta_x = newEnd.x - start.x;
					double intersecDelta_y = newEnd.y - start.y;
					if(doubleAbs(intersecDelta_x) <= doubleAbs(portalMaxDelta_x) ||
					   doubleAbs(intersecDelta_y) <= doubleAbs(portalMaxDelta_y)){
						colisionWithBlue = true;
						colisionWithOrange = false;
						portalMaxDelta_x = intersecDelta_x;
						portalMaxDelta_y = intersecDelta_y;
					}
				}
			}
		} 		
	}

	//There is movement in the y axis
	if(dyPos ^ dyNeg){
		double border_y = (dyNeg || tile_size_y == 0) ? y : (tile_size_y * TILE_SIZE + y -1);

		//Going through all points that may collide with a wall in the x axis
		double border_x;
		for(border_x = x-1; border_x < tile_size_x * TILE_SIZE + x ; border_x += TILE_SIZE){
			//Current point segment points
			Point start = {max(border_x, x) , border_y};
			Point end = {max(border_x, x) + delta_x, border_y + delta_y};

			//Check intersection of segment for all rectangles of map
			for(i = 0; i < curMap.nbMapWalls; i++) {
				Point newEnd = colisionResolvePlayer(start, end, curMap.wallList[i].rect);
				if (newEnd.y != -1 && (int)(newEnd.y) != (int)(end.y)) { //If collision on the y axis
					double intersecDelta_y = newEnd.y - start.y;
					maxDelta_y = doubleAbs(maxDelta_y) < doubleAbs(intersecDelta_y) ? maxDelta_y : intersecDelta_y;
					speed_y = 0;
				}
			}

			//Check intersection of segments for portals
			if(orange_portal.active){
				Point newEnd = colisionResolvePlayerPortal(start, end, orangeSegment);
				if (newEnd.y != -1){ //If colision
					double intersecDelta_x = newEnd.x - start.x;
					double intersecDelta_y = newEnd.y - start.y;
					if(doubleAbs(intersecDelta_x) <= doubleAbs(portalMaxDelta_x) ||
					   doubleAbs(intersecDelta_y) <= doubleAbs(portalMaxDelta_y)){
						colisionWithOrange = true;
						colisionWithBlue = false;
						portalMaxDelta_x = intersecDelta_x;
						portalMaxDelta_y = intersecDelta_y;
					}
				}
			}
			if(blue_portal.active){
				Point newEnd = colisionResolvePlayerPortal(start, end, blueSegment);
				if (newEnd.y != -1){ //If colision
					double intersecDelta_x = newEnd.x - start.x;
					double intersecDelta_y = newEnd.y - start.y;
					if(doubleAbs(intersecDelta_x) <= doubleAbs(portalMaxDelta_x) ||
					   doubleAbs(intersecDelta_y) <= doubleAbs(portalMaxDelta_y)){
						colisionWithBlue = true;
						colisionWithOrange = false;
						portalMaxDelta_x = intersecDelta_x;
						portalMaxDelta_y = intersecDelta_y;
					}
				}
			}	
		} 		
	}
	Point endPos = {x + maxDelta_x, y + maxDelta_y};

	//Player has colided with a portal before it has colided with a wall
	if((doubleAbs(portalMaxDelta_x) <= doubleAbs(maxDelta_x) 
	|| doubleAbs(portalMaxDelta_y) <= doubleAbs(maxDelta_y))
	&& (colisionWithBlue || colisionWithOrange) &&
	orange_portal.active && blue_portal.active){
		//Calculate the remaining distance the player has to travel 
		double newDelta_x = delta_x - portalMaxDelta_x;
		double newDelta_y = delta_y - portalMaxDelta_y;
		//Teleport the player using the portals
		Point newStart = teleportPlayer(colisionWithBlue ? blue_portal : orange_portal, 
										colisionWithOrange ? blue_portal : orange_portal,
										&newDelta_x, &newDelta_y);
		//Check for new colisions
		endPos = playerCollision(newStart.x, newStart.y,
								newDelta_x, newDelta_y,
								tile_size_x, tile_size_y);
	}else{
		player_speed.x = speed_x;
		player_speed.y = speed_y;
	}
	return endPos;
}

//Creates a new portal projectile with a color in argument
void create_portal_projectile(Portal_color color){
	//Check if we can create one
	if(!(nb_projectile_active < MAX_NUMBER_PROJECTILES)){
		return;
	}
	//create a portal projectile
	Portal_Projectile* proj = NULL;
	while(proj == NULL){
		proj = calloc(1, sizeof(Portal_Projectile));
	}
	//Place the projectile on the center of the player
	proj->pos.x = player_pos.x + TILE_SIZE;
	proj->pos.y = player_pos.y + TILE_SIZE;

	//Give the direction as the angle of the portal gun
	proj->speed.x = cos(portal_gun_angle_rand) * PORTAL_DELTA;
	proj->speed.y = sin(portal_gun_angle_rand) * PORTAL_DELTA;

	proj->color = color;

	//Add it to the list
	portal_projectile_list[nb_projectile_active] = proj;
	nb_projectile_active += 1;
}

//Handle all inputs from the player
void handlePlayerInput(){
	scanKeys();
	u16 keys = keysHeld();
	
	//Is the player in the air
	if(!isPlayerHittingWall((int)(player_pos.x), (int)(player_pos.y), Down)){
    	player_dir_y = Up;
    }else{
    	player_dir_y = None;
    }
	//If they are in the air and press down,
	//they will slow down time by lowering the time elapsed (which is delta_time). 
	if((keys & KEY_DOWN) && player_dir_y == Up) {
    	delta_time = DELTA_TIME_CONST / SLOW_DOWN_TIME_BY;
    }else{
    	delta_time = DELTA_TIME_CONST;
    }
    //Manage character movement, 
	//the player moves considerately less while in the air.
    if((keys & KEY_RIGHT) && (player_speed.x < MAX_RUNNING_SPEED)) {
        player_speed.x += (player_dir_y == None ?
        					PRESS_SPEED : PRESS_SPEED/AIR_SLOW_DOWN)* delta_time;
    }
    if((keys & KEY_LEFT) && (player_speed.x  > -MAX_RUNNING_SPEED)) {
        player_speed.x -= (player_dir_y == None ?
							PRESS_SPEED : PRESS_SPEED/AIR_SLOW_DOWN)* delta_time;
    }

	//The player can jump while they are on the ground
    if((keys & KEY_UP) && player_dir_y == None) {
        player_speed.y = JUMP_SPEED;
    }

	//If the player touchs the bottom touch screen,
	//they will rotate the bottom portal gun and change its angle.
    keys = keysDown();
    if(keys & KEY_TOUCH){
       touchPosition touch;
       touchRead(&touch);
       if(touch.px | touch.py){
    	   portal_gun_angle_rand = rotate_sub_bg2_for_gameplay(touch.px, touch.py);
    	   if(M_PI/2 < portal_gun_angle_rand && portal_gun_angle_rand < 3*M_PI/2){
    		   player_dir_x = Left;
    	   }else{
    		   player_dir_x = Right;
    	   }
       }
    }

	//if the player touches the back trigger,
	//it will cause a portal projectile to be created.
    if(keys & KEY_R){
    	//create orange portal projectile
		play_effect(SFX_SHOOT_ORANGE);
    	create_portal_projectile(Orange);	
    }else if(keys & KEY_L){
    	//create blue portal projectile
		play_effect(SFX_SHOOT_BLUE);
    	create_portal_projectile(Blue);
    }
	//restart the level by pressing start.
	if(keys & KEY_START){
		init_player();
	}
}

//Depending on the current player's direction and sprite state,
//update their sprite graphic and position.
void updatePlayerSprite(){

	//In the air:
	if(player_dir_y == Up){
		chell_sprite_state = JUMP;
	}//Not moving
	else if(doubleAbs(player_speed.x) < 1 && chell_sprite_state == IDLE){
		chell_sprite_state = IDLE;
	}//Do running animation
	else if(update_sprite_counter == 0){
		chell_sprite_state = chell_walk_cycle[walk_cycle_state];
		walk_cycle_state = (walk_cycle_state + 1) % 8;
	}


	//Set chell
	oamSet(&oamMain, 	// oam handler
			CHELL_SPRITE_ID,				// Number of sprite
    		(int)player_pos.x, (int)player_pos.y,			// Coordinates
    		0,				// Priority
    		0,				// Palette to use
    		SpriteSize_16x16,			// Sprite size
    		SpriteColorFormat_256Color,	// Color format
    		chell_sprites[chell_sprite_state],	// Loaded graphic to display
    		-1,				// Affine rotation to use (-1 none)
    		false,			// Double size if rotating
    		false,			// Hide this sprite
    		player_dir_x == Left, false,	// Horizontal or vertical flip
    		false			// Mosaic
    		);

	//Set portal gun
	//Rotation doesn't seem to work with flips sadly...
	//oamRotateScale(&oamMain, 0, degreesToAngle(-portal_gun_angle_rand*180/M_PI), intToFixed(1, 8), intToFixed(1, 8));
	oamSet(&oamMain, 	// oam handler
			GUN_SPRITE_ID,				// Number of sprite
    		(int)player_pos.x, (int)player_pos.y,			// Coordinates
    		0,				// Priority
    		0,				// Palette to use
    		SpriteSize_16x16,			// Sprite size
    		SpriteColorFormat_256Color,	// Color format
    		chell_sprites[GUN],	// Loaded graphic to display
    		0-1,				// Affine rotation to use (-1 none)
    		false,			// Double size if rotating
    		false,			// Hide this sprite
    		player_dir_x == Left, false,	// Horizontal or vertical flip
    		false			// Mosaic
    		);
} 

//Update the portal sprites depending on their position and the direction of the surface.
void placePortalSprites(TilePoint tilePoint, Direction dir, u16* gfx, int id){
	int max_i = (dir == Up) || (dir == Down) ? PORTAL_SIZE_PIXELS/TILE_SIZE : 1;
	int max_j = ((dir == Right) || (dir == Left)) ?  PORTAL_SIZE_PIXELS/TILE_SIZE : 1;
	int angle = (dir == Up) ?  degreesToAngle(270) :
				(dir == Right) ?  degreesToAngle(180) :
				(dir == Down) ?  degreesToAngle(90) : 0;
	int x = tilePoint.x * TILE_SIZE;
	int y = tilePoint.y * TILE_SIZE;


	//Rotate the portal so that it is parallel to the surface
	oamRotateScale(&oamMain, id, angle, intToFixed(1, 8), intToFixed(1, 8));
	int i;
	int j;
	for(i = 0; i < max_i; i+= 1){
		for(j = 0; j < max_j; j+= 1){

			oamSet(&oamMain, 	// oam handler
					    		id + i + j,				// Number of sprite
					    		x + i * TILE_SIZE,
					    		y + j * TILE_SIZE,			// Coordinates
					    		0,				// Priority
					    		0,				// Palette to use
					    		SpriteSize_8x8,			// Sprite size
					    		SpriteColorFormat_256Color,	// Color format
					    		gfx,	// Loaded graphic to display
					    		id,				// Affine rotation to use (-1 none)
					    		false,			// Double size if rotating
					    		false,			// Hide this sprite
					    		false, false,	// Horizontal or vertical flip
					    		false			// Mosaic
					    		);
		}
	}
}

//Depending on the animation state or the state of the portal,
//update the portal sprites.
void updatePortalSprites(){
	//We update the graphic only once every few frames.
	if(update_sprite_counter == 0){
			blue_portal.cycle_state = (blue_portal.cycle_state + 1) % 4;
			orange_portal.cycle_state = (orange_portal.cycle_state + 1) % 4;
	}
	//If the orange portal is active then we show it on screen
	if(orange_portal.active){
			placePortalSprites(orange_portal.topLeft,
								orange_portal.dir,
								orange_portal_sprites[portal_cycle[orange_portal.cycle_state]],
								ORANGE_PORTAL_BASE_SPRITE_ID);
	}else{
		oamClear(&oamMain, ORANGE_PORTAL_BASE_SPRITE_ID, 3);
	}
	//If the blue portal is active then we show it on screen
	if(blue_portal.active){
			placePortalSprites(blue_portal.topLeft,
								blue_portal.dir,
								blue_portal_sprites[portal_cycle[blue_portal.cycle_state]],
								BLUE_PORTAL_BASE_SPRITE_ID);
	}else{
		oamClear(&oamMain, BLUE_PORTAL_BASE_SPRITE_ID, 3);
	}
}

//Returns true if the othe portal is on the Tile
bool isOtherPortalOnTile(TilePoint tilePoint, Portal_color color){
	Portal otherPortal = (color == Blue) ? orange_portal : blue_portal;
	if(otherPortal.active == false){return false;}
	
	int rest_of_tiles_dir_x = (otherPortal.dir == Up || otherPortal.dir == Down);
	int rest_of_tiles_dir_y = (otherPortal.dir == Right || otherPortal.dir == Left);

	TilePoint tile1 = otherPortal.topLeft;
	TilePoint tile2 = {tile1.x + rest_of_tiles_dir_x, tile1.y + rest_of_tiles_dir_y};
	TilePoint tile3 = {tile2.x + rest_of_tiles_dir_x, tile2.y + rest_of_tiles_dir_y};

	//We get the 3 Tiles where the other portal is and we check if our tile is the same.
	return (tile1.x == tilePoint.x && tile1.y == tilePoint.y) ||
		   (tile2.x == tilePoint.x && tile2.y == tilePoint.y) ||
		   (tile3.x == tilePoint.x && tile3.y == tilePoint.y);
}

//Returns true if the tile is valid space to palce one tile of the portal
bool isTileValidForPortal(TilePoint tilePoint, Direction dir, Portal_color color){
	int disp_x = (dir == Left) ? -1 : (dir == Right) ? 1 : 0;
	int disp_y = (dir == Up) ? -1 : (dir == Down) ? 1 : 0;
	enum Tile airTile = getTile(tilePoint.x, tilePoint.y);
	enum Tile surfaceTile = getTile(tilePoint.x + disp_x, tilePoint.y + disp_y);
	return (airTile == Empty) && (surfaceTile == Wall) && !isOtherPortalOnTile(tilePoint, color);
}

//Returns true if a portal can be placed on these Tiles in this direction.
//We check some surrounding tiles in our directional axis to see if we can place a portal.
bool canPlacePortal(TilePoint* tilePoint, Direction dir, Portal_color color){
	if(!isTileValidForPortal(*tilePoint, dir, color)){return false;}

	int search_dir_x = (dir == Up || dir == Down);
	int search_dir_y = (dir == Left || dir == Right);

	TilePoint right1 = {tilePoint->x + search_dir_x, tilePoint->y + search_dir_y};
	bool isRight1Valid = isTileValidForPortal(right1, dir, color);

	TilePoint left1 = {tilePoint->x - search_dir_x, tilePoint->y - search_dir_y};
	bool isLeft1Valid = isTileValidForPortal(left1, dir, color);

	if(!isRight1Valid && !isLeft1Valid){return false;}

	if(isRight1Valid && isLeft1Valid){
		tilePoint->x = left1.x;
		tilePoint->y = left1.y;
		return true;
	}

	if(isRight1Valid){
		TilePoint right2 = {tilePoint->x + 2*search_dir_x, tilePoint->y + 2*search_dir_y};
		if(!isTileValidForPortal(right2, dir, color)){return false;}
	}else{
		TilePoint left2 = {tilePoint->x - 2*search_dir_x, tilePoint->y - 2*search_dir_y};
		if(!isTileValidForPortal(left2, dir, color)){return false;}
		tilePoint->x = left2.x;
		tilePoint->y = left2.y;
	}
	return true;
}

//Updates the position of the projectile,
//in case of collision it deallocates the projectile and spawns a portal.
void updateProjectile(Portal_Projectile* proj, int id) {
	//A priori if no collision on path
	double delta_x = proj->speed.x * delta_time;
	double delta_y = proj->speed.y * delta_time;
	
	//Current point segment points
	Point start = {proj->pos.x , proj->pos.y};
	Point end = {proj->pos.x + delta_x, proj->pos.y + delta_y};
	Direction collidedDir = None;

	//Check intersection of segment for all rectangles of map
	int i;
	for(i = 0; i < curMap.nbMapWalls; i++) {
		Direction dir = None;
		Point newEnd = colisionResolveProjectile(start, end, curMap.wallList[i].rect, &dir);
		if (newEnd.x != -1) { //If collision
			double intersecDelta_y = newEnd.y - start.y;
			double intersecDelta_x = newEnd.x - start.x;
			if(doubleAbs(intersecDelta_y) <= doubleAbs(delta_y) ||
			   doubleAbs(intersecDelta_x) <= doubleAbs(delta_x)){
				delta_x =  intersecDelta_x;
				delta_y =  intersecDelta_y;
				collidedDir = dir;
			}
		}
	}
	//If we have a collision
	if(collidedDir != None){
		Point point = {start.x + delta_x, start.y + delta_y};
		TilePoint tilePoint = getTilePoint(point);
		//We spawn the portal if the tiles are valid surfaces.
		if(canPlacePortal(&tilePoint, collidedDir, proj->color)){
			play_effect(SFX_PORTAL_OPEN);
			Portal* portal = (proj->color == Blue) ? &blue_portal : &orange_portal;
			portal->active = true;
			portal->topLeft.x = tilePoint.x;
			portal->topLeft.y = tilePoint.y;
			portal->dir = collidedDir;
		}else{
			play_effect(SFX_PORTAL_INVALID_SURFACE);
		}
		//Deallocates the projectile.
		deleteProjectile(proj, id);

	}else{
		//Otherwise we update the position of the projectile and show it.
		proj->pos.x += delta_x;
		proj->pos.y += delta_y;

		u16* gfx = (proj->color) == Blue ? blue_portal_sprites[PROJECTILE] : orange_portal_sprites[PROJECTILE];
		oamSet(&oamMain, 	// oam handler
				PORJECTILE_BASE_SPRITE_ID + id,				// Number of sprite
				proj->pos.x - TILE_SIZE,				    	
				proj->pos.y - TILE_SIZE,			// Coordinates
	    		0,				// Priority
				0,				// Palette to use
				SpriteSize_8x8,			// Sprite size
				SpriteColorFormat_256Color,	// Color format
				gfx,	// Loaded graphic to display
				id,				// Affine rotation to use (-1 none)
				false,			// Double size if rotating
				false,			// Hide this sprite
				false, false,	// Horizontal or vertical flip
				false			// Mosaic
				);
	}
}

//Updates the position of all active pojectiles
void updatePortalProjectiles() {
	int i;
	for(i = 0; i < MAX_NUMBER_PROJECTILES; i++){
		Portal_Projectile* proj = portal_projectile_list[i];
		if(proj != NULL){
			updateProjectile(proj, i);
		}
	}
}

//Updates the player's variables 
void updatePlayer() {
	//handle the player's inputs and update the projectiles
	handlePlayerInput();
	updatePortalProjectiles();

	//Enforce gravity on the player
	player_speed.y += GRAVITY_CONST * delta_time;

	//Enforce drag on the player
	if(player_dir_y == None){
		player_speed.x += -DRAG_CONST * player_speed.x * delta_time;
	}else{
		player_speed.x += -DRAG_CONST/DRAG_AIR_SLOW_DOWN * player_speed.x * delta_time;
	}
	//Make it go back to 0 if it is close enough
	if(doubleAbs(player_speed.x) < 0.01){
		player_speed.x = 0;
	}

    //Update player's position depending on his speed and direction
    double delta_y = (delta_time * player_speed.y);
    double delta_x = (delta_time * player_speed.x);
    Point newPoint = playerCollision(player_pos.x, player_pos.y, delta_x, delta_y, PLAYER_SIZE, PLAYER_SIZE);

    player_pos.x = newPoint.x;
    player_pos.y = newPoint.y;

	//This is to avoid having 2 sounds in quick secession, we limit it to 1 per VBlank
    if(has_gone_through_a_portal){
    	has_gone_through_a_portal = false;
    	play_effect(SFX_PORTAL_ENTER);
    }
}

//Update the sprites
void updateSprites() {
	updatePlayerSprite();
	updatePortalSprites();

	oamUpdate(&oamMain);
	update_sprite_counter = (update_sprite_counter + 1) % UPDATE_SPRITES;
}

//Deallocates all sprites ressources from VRAM memory
void deallocateRessources(){
	init_player();

	int i;
	for(i = 0; i < NB_CHELL_SPRITES; i ++){
		//Deallocate the space of the chell sprites
		oamFreeGfx(&oamMain, chell_sprites[i]);
	}
	for(i = 0; i < NB_PORTAL_SPRITES; i ++){
		//Deallocate the space of the portals sprites
		oamFreeGfx(&oamMain, orange_portal_sprites[i]);

		oamFreeGfx(&oamMain, blue_portal_sprites[i]);
	}
}
