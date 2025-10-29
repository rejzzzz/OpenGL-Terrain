#pragma once

#include <string>
#include <vector>
#include <glm/vec3.hpp>

// Forward declaration for the Camera class
class Camera; 

/**
 * Loads the 6 skybox textures into a single cubemap.
 * ... (comments) ...
 */
void loadSkybox(const std::vector<std::string>& faces);

/**
 * Draws the skybox.
 * Call this every frame before drawing anything else.
 * @param camera A const reference to your scene's camera object.
 * @param center The 3D point the camera is looking at (the player).
 */
void drawSkybox(const Camera& camera, const glm::vec3& center);