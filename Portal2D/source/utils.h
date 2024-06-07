#include <nds.h>
#include "map.h"
#define CLOCKWISE 1
#define COUNTER_CLOCKWISE -1
typedef enum {Left, Up, Right, Down, None} Direction;

int min(int a, int b);
int max(int a, int b);
double doubleAbs(double x);

Point colisionResolvePlayer(Point start, Point end, Vector rec);
Point colisionResolveProjectile(Point start, Point end, Vector rec, Direction* dir);
Point colisionResolvePlayerPortal(Point start, Point end, Segment portal);

