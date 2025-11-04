#pragma once

#include <vector>

// Generate a simple city layout with houses, roads and street lights
// nHouses: number of houses to generate
// areaRadius: approximate radius around origin to place houses
#include <glm/vec2.hpp>

// generateCity: optionally accept a lake center so the caller (scene) can place the lake
void generateCity(int nHouses = 30, float areaRadius = 40.0f, const glm::vec2 &lakeCenter = glm::vec2(0.0f,0.0f));

// Clears any previously generated city elements and rebuilds with new parameters
void clearCity();
