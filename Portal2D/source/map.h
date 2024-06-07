#include <nds.h>

#define TILE_SIZE 8

#define DOOR_TILE_SIZE 3
enum Tile {Empty, Wall, Forbidden, Button, ButtonGround, DoorTopLeftClose, DoorTopMiddleClose, DoorMiddleLeftClose, DoorMiddleClose, DoorTopLeftOpen, DoorTopMiddleOpen, DoorMiddleLeftOpen, DoorMiddleOpen, TopLeftGrey, StartingPoint};

//In pixel size
typedef struct {
	double x;
	double y;
} Point;

//In tile size
typedef struct {
	int x;
	int y;
} TilePoint;
typedef struct {
	TilePoint from; //top left corner of a rectangle or start of a segment
	TilePoint to; //bottom right corner of a rectangle or end of a segment
} Vector;
typedef struct {
	Vector rect;
	enum Tile type;
} WallType;
typedef struct {
	Point start;
	Point end;
} Segment;
typedef struct {
	TilePoint startingPoint;
	TilePoint button;
	TilePoint doorTopLeft;
	WallType* wallList;
	size_t nbMapWalls;
}Map;

Map curMap;

uint16 getTile(int x, int y);
bool isPointInBounds(Point point);
TilePoint getTilePoint(Point point);
bool are2TilesHard(enum Tile tile1, enum Tile tile2);
bool are3TilesHard(enum Tile tile1, enum Tile tile2, enum Tile tile3);
void openDoor();
void closeDoor();
void loadLevel1();
void loadLevel2();
void loadLevel3();
void loadLevel4();
void deallocateCurMap();
void loadBackground3Map();
