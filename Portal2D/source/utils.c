#include "utils.h"
#define CLOCKWISE 1
#define COUNTER_CLOCKWISE -1
Point noIntersection = {-1, -1};//Special point to return when function found no intersection

//Returs the minimum from a and b
int min(int a, int b){
	return a < b ? a : b;
}
//Returs the maximum from a and b
int max(int a, int b){
	return a > b ? a : b;
}
//Returns the squared distance from 2 points a and b
double sq_dist(Point a, Point b){
	return ((a.x) - (b.x)) * ((a.x) - (b.x)) + ((a.y) - (b.y)) * ((a.y) - (b.y));
}
//Returns the round up version of a double x
int roundUp(double x){
    int intPart = (int)x;
    return (x > intPart) ? intPart + 1 : intPart;
}
//Returns the absolute value of a double x
double doubleAbs(double x) {
    return (x < 0) ? -x : x;
}

//Returns orientation of points start, end, other
//0 for aligned points
//1 for clockwise orientation
//-1 for counterclockwise orientation
int getOrientation(Point start, Point end, Point other) {
	int orientation = (((int)(end.y) - (int)(start.y)) * ((int)(other.x) - (int)(end.x)) -
			(((int)(end.x) - (int)(start.x))) * ((int)(other.y) - (int)(end.y)));
	if (orientation == 0) return orientation; //3 points aligned
	return orientation > 0 ? CLOCKWISE : COUNTER_CLOCKWISE; //3 points not aligned, oriented clockwise or counterclockwise
}


//Returns true if point is included in segment start end
bool segmentContains(Point start, Point end, Point point) {
	return ((min((int)(start.x), (int)(end.x)) <= (int)(point.x)) && ((int)(point.x) <= max((int)(start.x), (int)(end.x))) && //Projection of third point on x axis is on the segment of the projection on x axis of segment start end
			(min((int)(start.y), (int)(end.y)) <= (int)(point.y)) && ((int)(point.y) <= max((int)(start.y), (int)(end.y)))); //Projection of third point on y axis is on the segment of the projection on y axis of segment start end

}

//Returns true if the first point is closest to start,
//Checks for out of bounds points but assumes that at least 1 is in bounds.
bool isFirstCloserToStart(Point start, Point point1, Point point2) {
	if(!isPointInBounds(point1)) return false;
	if(!isPointInBounds(point2)) return true;
	return sq_dist(start, point1) < sq_dist(start, point2);
}

//Returns the intersection point of 2 segement from start1 to end1 and start2 to end2.
//Segments are assumed to not be colinear as this case is already handled in doSegmentsIntersect
Point getIntersect(Point start1, Point end1, Point start2, Point end2) {
	double delta1_x = end1.x - start1.x;
	double delta1_y = end1.y - start1.y;
	double delta2_x = end2.x - start2.x;
	double delta2_y = end2.y - start2.y;
	double ratioIntersect = ((delta2_x * (start1.y - start2.y) - delta2_y * (start1.x - start2.x))) / (-delta2_x * delta1_y + delta1_x * delta2_y);
	Point intersection = {start1.x + ratioIntersect * delta1_x, start1.y + ratioIntersect * delta1_y};
	return intersection;
}


//Returns true if segment start1 end1 intersects with start2 end2,
//and if so gives the intersection point between the segments.
bool doSegmentsIntersect(Point start1, Point end1, Point start2, Point end2, Point* intersection) {
	//orientations computations
	int o1 = getOrientation(start1, end1, start2);
	int o2 = getOrientation(start1, end1, end2);
	int o3 = getOrientation(start2, end2, start1);
	int o4 = getOrientation(start2, end2, end1);

	//Intersection when points are not aligned
	if (o1 != o2 && o3 != o4){
		*intersection = getIntersect(start1, end1, start2, end2);
		return true;
	}

	//Intersection when points are aligned
	if (o1 == 0 && segmentContains(start1, end1, start2)){//start2 on segment start1 end1
		*intersection = start2;
		return true;
	}
	if (o2 == 0 && segmentContains(start1, end1, end2)) {//end2 on segment start1 end1
		*intersection = end2;
		return true;
	}
	if (o3 == 0 && segmentContains(start2, end2, start1)) { //start1 on segment start2 end2
		*intersection = start1;
		return true;
	}
	if (o4 == 0 && segmentContains(start2, end2, end1)) {//end1 on segment start2 end2
		*intersection = end1;
		return true;
	}

	//Segments don't intersect
	return false;
}

//Resolves a player collision from start to end with a rectangle,
//if no collision returns noIntersection.
Point colisionResolvePlayer(Point start, Point end, Vector rec) {
	//Convert the corners of the rectangle into pixels positions
	Point topLeftCorner = (Point){rec.from.x * TILE_SIZE, rec.from.y * TILE_SIZE};
	Point bottomLeftCorner = (Point){rec.from.x * TILE_SIZE, (rec.to.y + 1) * TILE_SIZE - 1};
	Point topRightCorner = (Point){(rec.to.x + 1) * TILE_SIZE - 1, rec.from.y * TILE_SIZE};
	Point bottomRightCorner = (Point){(rec.to.x + 1) * TILE_SIZE - 1, (rec.to.y + 1) * TILE_SIZE - 1};

	Point furthestPoint = noIntersection;
	Point intersectPoint;

	//For every segment in the rectangle we test for collision,
	//If we get one we only reduce the magnitude in the direction 
	//in which the collision happened to avoid a collision and keep momentum.
	
	//Case Left
	if (doSegmentsIntersect(start, end, topLeftCorner, bottomLeftCorner, &intersectPoint)) {
		Point resolvedColisionPoint = {topLeftCorner.x - 0.01, end.y};
		if(isFirstCloserToStart(start, resolvedColisionPoint, furthestPoint)){
			furthestPoint = resolvedColisionPoint;
		}
	}
	//Case Right
	if (doSegmentsIntersect(start, end, topRightCorner, bottomRightCorner, &intersectPoint)) {
		Point resolvedColisionPoint = {topRightCorner.x + 1, end.y};
		if(isFirstCloserToStart(start, resolvedColisionPoint, furthestPoint)){
			furthestPoint = resolvedColisionPoint;
		}
	}
	//Case Top
	if (doSegmentsIntersect(start, end, topLeftCorner, topRightCorner, &intersectPoint)) {
		Point resolvedColisionPoint = {end.x, topLeftCorner.y - 0.01};
		if(isFirstCloserToStart(start, resolvedColisionPoint, furthestPoint)){
			furthestPoint = resolvedColisionPoint;
		}
	}
	//Case Bottom
	if (doSegmentsIntersect(start, end, bottomLeftCorner, bottomRightCorner, &intersectPoint)) {
		Point resolvedColisionPoint = {end.x, bottomLeftCorner.y + 1};
		if(isFirstCloserToStart(start, resolvedColisionPoint, furthestPoint)){
			furthestPoint = resolvedColisionPoint;
		}
	}

	return furthestPoint;
}

//Resolves a projectile collision from start to end with a rectangle,
//if no collision returns noIntersection.
//The direction is used to find in which surface we need to place a portal
Point colisionResolveProjectile(Point start, Point end, Vector rec, Direction* dir) {
	//Convert the corners of the rectangle into pixels positions
	Point topLeftCorner = (Point){rec.from.x * TILE_SIZE, rec.from.y * TILE_SIZE};
	Point bottomLeftCorner = (Point){rec.from.x * TILE_SIZE, (rec.to.y + 1) * TILE_SIZE - 1};
	Point topRightCorner = (Point){(rec.to.x + 1) * TILE_SIZE - 1, rec.from.y * TILE_SIZE};
	Point bottomRightCorner = (Point){(rec.to.x + 1) * TILE_SIZE - 1, (rec.to.y + 1) * TILE_SIZE - 1};

	Point furthestPoint = noIntersection;
	Point intersectPoint;

	//For every segment in the rectangle we test for collision,
	//If we get one, we avoid the collision by placing the projectile 
	//at the closest free point next to the itersection point. 

	//Case left
	if (doSegmentsIntersect(start, end, topLeftCorner, bottomLeftCorner, &intersectPoint)) {
		Point resolvedColisionPoint = {intersectPoint.x  - 0.01, intersectPoint.y};
		if(isFirstCloserToStart(start, resolvedColisionPoint, furthestPoint)){
			furthestPoint = resolvedColisionPoint;
			*dir = Right;
		}
	}
	//Case right
	if (doSegmentsIntersect(start, end, topRightCorner, bottomRightCorner, &intersectPoint)) {
		Point resolvedColisionPoint = {intersectPoint.x + 1, intersectPoint.y};
		if(isFirstCloserToStart(start, resolvedColisionPoint, furthestPoint)){
			furthestPoint = resolvedColisionPoint;
			*dir = Left;
		}
	}
	//Case top
	if (doSegmentsIntersect(start, end, topLeftCorner, topRightCorner, &intersectPoint)) {
		Point resolvedColisionPoint = {intersectPoint.x, intersectPoint.y - 0.01};
		if(isFirstCloserToStart(start, resolvedColisionPoint, furthestPoint)){
			furthestPoint = resolvedColisionPoint;
			*dir = Down;
		}
	}
	//Case bottom
	if (doSegmentsIntersect(start, end, bottomLeftCorner, bottomRightCorner, &intersectPoint)) {
		Point resolvedColisionPoint = {intersectPoint.x, intersectPoint.y + 1};
		if(isFirstCloserToStart(start, resolvedColisionPoint, furthestPoint)){
			furthestPoint = resolvedColisionPoint;
			*dir = Up;
		}
	}

	return furthestPoint;
}

//Resolves a player portal collision from start to end with the portal segment,
//if no collision returns noIntersection.
Point colisionResolvePlayerPortal(Point start, Point end, Segment portalSegment){
	//We simply find the intersection point if there is one between these segments.
	Point intersectPoint = noIntersection;
	doSegmentsIntersect(start, end, portalSegment.start, portalSegment.end, &intersectPoint);
	return intersectPoint;
}
