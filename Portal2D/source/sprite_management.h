#include <nds.h>

void configureSprites();
void init_player();
void updatePlayer();
void updateSprites();
bool isPlayerNear(uint16 tiletype);
void deallocateRessources();

#define PLAYER_SIZE 2
#define PLAYER_SIZE_PX 16 //2 * 8
