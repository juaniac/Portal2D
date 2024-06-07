#include "map.h"
#include "graphics_main_engine.h"
#include <stdio.h>//debug

//Draws Tile tile at (x,y) x and y in TILE coordinates 
void putTile(TilePoint pos, uint16 tile){
	int x = pos.x, y = pos.y;
	BG_MAP_RAM(MAP_RAM_B2_OFFSET)[y * TILE_MAP_WIDTH + x] = tile;
}

//Draws a rectangle of full of tiles from corner from to corner until included
void fillRectangle(TilePoint from, TilePoint until, uint16 tile){
	int left = from.x, top = from.y, right = until.x, bottom = until.y;
	if (!((0 <= top) && (top < SCREEN_HEIGHT) && (0 <= bottom) && (bottom < SCREEN_HEIGHT) &&
        (0 <= right) && (right < SCREEN_WIDTH) && (0 <= left) && (left < SCREEN_WIDTH))){return;}//Check rectangle included in screen
	int row, col;
	for (row = top; row <= bottom; row++) {
    	for (col = left; col <= right; col++) {
            TilePoint pos = {col, row};
    		putTile(pos, tile);
    	}
    }
}

//Allocates an element on the map, toX and toY not used for button.
void addMapElement(int fromTileX, int fromTileY, int toTileX, int toTileY, enum Tile tile){
    //If the tile type is a rectangle wall, then we must add it to our map and draw it on the screen.
    if(tile == Wall || tile == Forbidden){
        TilePoint wallTopLeft = {fromTileX, fromTileY};
        TilePoint wallBottomRight = {toTileX, toTileY};
        Vector rect = {wallTopLeft, wallBottomRight};
        fillRectangle(wallTopLeft, wallBottomRight, tile);
        
        curMap.nbMapWalls += 1;
        curMap.wallList = realloc(curMap.wallList, curMap.nbMapWalls * sizeof(WallType));

        curMap.wallList[curMap.nbMapWalls - 1].rect = rect;
        curMap.wallList[curMap.nbMapWalls - 1].type = tile;
    }
    //If the tile type is a button, then we must add it to our map and draw it on the screen.
    else if(tile == Button){
        TilePoint button = {fromTileX, fromTileY};
        putTile(button, Button);
        curMap.button = button;
    }
    //If the tile type is a door, then we must add it to our map and draw it as it is closed.
    else if(tile == DoorTopLeftClose){
        TilePoint doorTopLeft = {fromTileX, fromTileY};
        curMap.doorTopLeft = doorTopLeft;
        closeDoor();
    }
    //If the tile type is the player starting point, then we simply add it to the map.
    else if(tile == StartingPoint){
        TilePoint startingPoint = {fromTileX, fromTileY};
        curMap.startingPoint = startingPoint;
    }
}

//Put to 0 all map variables and clear the VRAM
void initCurMap(){
	TilePoint initial = {0,0};
    curMap.startingPoint = initial;
    curMap.button = initial;
    curMap.doorTopLeft = initial;
    curMap.wallList = NULL;
    curMap.nbMapWalls = 0;
    TilePoint wallTopLeft = {0, 0};
    TilePoint wallBottomRight = {31, 23};
    fillRectangle(wallTopLeft, wallBottomRight, Empty);
}

//Level 1 definition
void loadLevel1() {
    initCurMap();

    addMapElement(0, 0, 31, 1, Wall);
    addMapElement(0, 2, 1, 23, Wall);
    addMapElement(2, 22, 31, 23, Wall);
    addMapElement(30, 2, 31, 23, Wall);

    addMapElement(10, 21, 29, 21, Forbidden);
    addMapElement(12, 20, 29, 20, Forbidden);
    addMapElement(14, 19, 29, 19, Forbidden);
    addMapElement(16, 18, 29, 18, Forbidden);
    addMapElement(18, 17, 29, 17, Forbidden);
    addMapElement(20, 16, 29, 16, Forbidden);
    addMapElement(22, 15, 29, 15, Forbidden);
    addMapElement(24, 14, 29, 14, Forbidden);

    addMapElement(6, 21, 6, 21, Button);
    addMapElement(27, 11, 27 + DOOR_TILE_SIZE -1, 11 + DOOR_TILE_SIZE -1, DoorTopLeftClose);

    addMapElement(2, 20, 0, 0, StartingPoint);
}

//Level 2 definition
void loadLevel2(){
    initCurMap();
    
    addMapElement(0, 0, 7, 0, Wall);
    addMapElement(8, 0, 31, 0, Forbidden);
    addMapElement(31, 1, 31, 12, Forbidden);
    addMapElement(0, 5, 7, 23, Wall);
    addMapElement(24, 13, 31, 13, Forbidden);
    addMapElement(24, 14, 31, 23, Wall);
    addMapElement(8, 22, 23, 23, Wall);
    addMapElement(0, 0, 0, 4, Wall);

    addMapElement(4, 4, 4, 4, Button);
    addMapElement(27, 10, 27 + DOOR_TILE_SIZE -1, 10 + DOOR_TILE_SIZE -1, DoorTopLeftClose);

    addMapElement(1, 3, 0, 0, StartingPoint);
}

//Level 3 definition
void loadLevel3(){
    initCurMap();
    
    addMapElement(0, 0, 0, 23, Wall);
    addMapElement(0, 23, 17, 23, Wall);
    addMapElement(18, 10, 19, 23, Forbidden);
    addMapElement(20, 23, 31, 23, Forbidden);
    addMapElement(31, 0, 31, 23, Forbidden);
    addMapElement(1, 0, 31, 0, Forbidden);

    addMapElement(17, 22, 17, 22, Button);
    addMapElement(28, 20, 28 + DOOR_TILE_SIZE -1, 20 + DOOR_TILE_SIZE -1, DoorTopLeftClose);

    addMapElement(1, 21, 0, 0, StartingPoint);
}

 //Level 4 definition
void loadLevel4(){
    initCurMap();
    
    addMapElement(0, 0, 0, 23, Forbidden);
    addMapElement(0, 0, 31, 0, Forbidden);
    addMapElement(31, 0, 31, 23, Forbidden);
    addMapElement(1, 23, 30, 23, Wall);

    addMapElement(1, 1, 1, 14, Wall);
    addMapElement(0, 5, 3, 5, Forbidden);

    addMapElement(0, 10, 3, 10, Forbidden);

    addMapElement(8, 5, 11, 5, Forbidden);
    addMapElement(7, 10, 11, 10, Forbidden);
    addMapElement(10, 5, 10, 15, Forbidden);
    addMapElement(7, 15, 11, 15, Forbidden);

    addMapElement(15, 0, 15, 4, Forbidden);
    addMapElement(16, 1, 16, 4, Wall);

    addMapElement(26, 7, 31, 7, Forbidden);

    addMapElement(10, 4, 10, 4, Button);
    addMapElement(27, 4, 27 + DOOR_TILE_SIZE -1, 4 + DOOR_TILE_SIZE -1, DoorTopLeftClose);

    addMapElement(1, 21, 0, 0, StartingPoint);
}

//deallocates all walls from map.
void deallocateCurMap(){
    if(curMap.wallList != NULL){
        free(curMap.wallList);
        curMap.nbMapWalls = 0;
    }
}

//check if this point is inside the screen.
bool isPointInBounds(Point point){
    return (0 <= point.x && point.x < SCREEN_WIDTH && 0 <= point.y && point.y < SCREEN_HEIGHT);
}

//Returns the tile type of the position (x,y) tile from tilemap
uint16 getTile(int x, int y) {
    if(!(0 <= x && x < SCREEN_WIDTH/TILE_SIZE && 0 <= y && y < SCREEN_HEIGHT/TILE_SIZE)) 
        return Wall;
    //Mask the 10 lower bits as those are the ones that are result f
	return (BG_MAP_RAM(MAP_RAM_B2_OFFSET)[y * TILE_MAP_WIDTH + x]) & 0b1111111111;
}

//Returns the Tile point from pixel point given 
TilePoint getTilePoint(Point point){
    TilePoint tilePoint = {(int)(point.x)/TILE_SIZE, (int)(point.y)/TILE_SIZE};
    return tilePoint;
}

//Returns true if tile1 or tile2 is hard (wall or forbidden)
bool are2TilesHard(enum Tile tile1, enum Tile tile2) {
	return ((tile1 == Wall) | (tile1 == Forbidden) |
			(tile2 == Wall) | (tile2 == Forbidden));
}

//Returns true if tile1 or tile2 or tile3 is hard (wall or forbidden)
bool are3TilesHard(enum Tile tile1, enum Tile tile2, enum Tile tile3) {
	return ((tile1 == Wall) | (tile1 == Forbidden) |
			(tile2 == Wall) | (tile2 == Forbidden) |
			(tile3 == Wall) | (tile3 == Forbidden));
}

//Load the Tiled background 3 in the VRAM
void loadBackground3Map(){
    int row, col;
	for (row = 0; row < TILE_MAP_HEIGHT; row++) {
    	for (col = 0; col < TILE_MAP_WIDTH; col++) {
    		BG_MAP_RAM(MAP_RAM_B3_OFFSET)[row * TILE_MAP_WIDTH + col] = TopLeftGrey |
                                                    ((col % 2 == 1) ? TILE_FLIP_H : 0) |
                                                    ((row % 2 == 1) ? TILE_FLIP_V : 0);
    	}
    }
}

//Open the tiled door
void openDoor(){
    int delta_x;
    int delta_y;
    for(delta_x = 0; delta_x < DOOR_TILE_SIZE ; delta_x += 1){
        for(delta_y = 0; delta_y < DOOR_TILE_SIZE ; delta_y += 1){
            TilePoint tile = {curMap.doorTopLeft.x + delta_x, curMap.doorTopLeft.y + delta_y};
            uint16 tileType = (((delta_x % (DOOR_TILE_SIZE-1) == 0) && (delta_y % (DOOR_TILE_SIZE-1) == 0)) ? DoorTopLeftOpen :
                                (delta_x % (DOOR_TILE_SIZE-1) == 0) ? DoorMiddleLeftOpen :
                                (delta_y % (DOOR_TILE_SIZE-1) == 0) ? DoorTopMiddleOpen :
                                DoorMiddleOpen) |
                            ((delta_x == DOOR_TILE_SIZE-1) ? TILE_FLIP_H : 0) |
                            ((delta_y == DOOR_TILE_SIZE-1) ? TILE_FLIP_V : 0);
            putTile(tile, tileType);

        }
    }
}

//Close the tiled door
void closeDoor(){
    int delta_x;
    int delta_y;
    for(delta_x = 0; delta_x < DOOR_TILE_SIZE ; delta_x += 1){
        for(delta_y = 0; delta_y < DOOR_TILE_SIZE ; delta_y += 1){
            TilePoint tile = {curMap.doorTopLeft.x + delta_x, curMap.doorTopLeft.y + delta_y};
            uint16 tileType = (((delta_x % (DOOR_TILE_SIZE-1) == 0) && (delta_y % (DOOR_TILE_SIZE-1) == 0)) ? DoorTopLeftClose :
                                (delta_x % (DOOR_TILE_SIZE-1) == 0) ? DoorMiddleLeftClose :
                                (delta_y % (DOOR_TILE_SIZE-1) == 0) ? DoorTopMiddleClose :
                                DoorMiddleClose) |
                                ((delta_x == DOOR_TILE_SIZE-1) ? TILE_FLIP_H : 0) |
                                ((delta_y == DOOR_TILE_SIZE-1) ? TILE_FLIP_V : 0);
            putTile(tile, tileType);
        }
    }
}
