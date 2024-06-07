#include <nds.h>
#define TILE_MAP_WIDTH 32
#define TILE_MAP_HEIGHT 24
#define MAP_RAM_B2_OFFSET 0
#define MAP_RAM_B3_OFFSET 2

void init_main_for_gameplay();
void init_main_startup();
void init_main_ending();
bool isPlayerTouchingGround(int x, int y);
bool PlayerFitScreen();
