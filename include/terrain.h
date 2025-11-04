// Terrain public API (immediate-mode implementation)
#pragma once

#include <glm/glm.hpp>

// Returns height of terrain at world position (x, z)
float getTerrainHeight(float x, float z);

// Returns the base terrain height (hills + mountains) without any pond deformation.
float getTerrainBaseHeight(float x, float z);

// Renders the terrain mesh (immediate mode for now)
void drawTerrain();

// Configure procedural "mountains" that add on top of base height
// center = (x,z) in world units within the terrain range, radius in world units, height in world units
void terrainAddMountain(const glm::vec2& center, float radius, float height);
void terrainClearMountains();
