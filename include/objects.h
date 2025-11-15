#ifndef OBJECTS_H
#define OBJECTS_H
// include/objects.h

#include <glm/vec3.hpp>
#include <glm/vec2.hpp>
#include <vector>
#include <string>

// Building description used by the scene and city generator
// type: 0 = no texture, 1 = brick, 2 = metal
struct BuildingDef { float x; float z; float bw; float bh; float bd; glm::vec3 windowColor; int type = 0; };

void drawBuildings();
void drawTrees();
void drawRoads();

// Road type and management (polyline in X,Z world coords)
struct Road { std::vector<glm::vec2> pts; float halfWidth = 1.2f; bool isMain = false; };
void addRoad(const Road &r);
void clearRoads();

// Ponds (simple circular lakes)
void addPond(const glm::vec2 &center, float radius);
void clearPonds();
void drawPonds();
// Accessor so terrain can see pond definitions for basin carving
const std::vector<std::pair<glm::vec2,float>>& getPonds();

// Coins (collectibles)
void spawnCoins(int n, float areaRadius);
void clearCoins();
void drawCoins();
// Check player position (x,z) for coin pickup within radius; returns number collected this check
int collectCoinsAt(float x, float z, float pickupRadius = 0.8f);
int getCollectedCoinsCount();
int getTotalCoinsCount();

// Street lights
void addStreetLight(const glm::vec3 &pos);
void clearStreetLights();
void drawStreetLights();

// Add or clear buildings (used by city generator)
void addBuilding(const BuildingDef &b);
void clearBuildings();

// Init building textures from file paths (type 1=brick, type 2=metal)
void initBuildingTextures(const std::string &brickPath, const std::string &metalPath);

// Accessor for buildings
const std::vector<BuildingDef>& getBuildings();

// Accessor for roads
const std::vector<Road>& getRoads();

// Accessor for trees
const std::vector<glm::vec2>& getTrees();

// Accessor for street lights
const std::vector<glm::vec3>& getStreetLights();

// Accessor for coins
const std::vector<glm::vec2>& getCoins();

// Collision query: returns true if a circle centered at (x,z) with given radius
// would intersect any building footprint. Used to prevent player entering buildings.
bool isPositionInsideBuilding(float x, float z, float radius);

#endif