#pragma once

#include <glm/glm.hpp>
#include "terrain.h"
// include shader header if you have one
#include "shader.h" // adjust path/name if different

class BuildingObject {
public:
    BuildingObject(const glm::vec3& pos, float scale, Terrain* terrain)
        : position(pos), scale(scale), terrain(terrain) {
        // place building on terrain surface
        position.y = terrain->getHeightAt(position.x, position.z);
    }

    // draw must be called from your render loop with an active shader
    void draw(Shader &shader);

    // optional: update (if you want to re-align during runtime)
    void update() {
        position.y = terrain->getHeightAt(position.x, position.z);
    }

private:
    glm::vec3 position;
    float scale;
    Terrain* terrain;

    // simple cube VAO/VBO static to reuse across buildings
    static unsigned int cubeVAO;
    static unsigned int cubeVBO;
    static void initCube();
};
