#ifndef OBJECTS_H
#define OBJECTS_H
// include/objects.h

void drawBuildings();
void drawTrees();
void drawRoads();

// Collision query: returns true if a circle centered at (x,z) with given radius
// would intersect any building footprint. Used to prevent player entering buildings.
bool isPositionInsideBuilding(float x, float z, float radius);

#endif