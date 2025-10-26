#include <GL/glew.h>
#include <cmath>
#include <vector>
#include <algorithm>
#include <glm/glm.hpp>
#include "terrain.h"

// Internal mountain data
struct Mountain {
    glm::vec2 center; // (x, z)
    float radius;
    float height;
};

static std::vector<Mountain> s_mountains;

void terrainAddMountain(const glm::vec2& center, float radius, float height) {
    s_mountains.push_back(Mountain{center, radius, height});
}

void terrainClearMountains() {
    s_mountains.clear();
}

// Base rolling hills + optional mountain domes
float getTerrainHeight(float x, float z) {
    // Base gentle hills
    float y = 0.5f * std::sin(x * 0.2f) * std::cos(z * 0.2f);

    // Add mountains with smooth dome falloff
    for (const auto& m : s_mountains) {
        float dx = x - m.center.x;
        float dz = z - m.center.y;
        float dist = std::sqrt(dx*dx + dz*dz);
        if (dist < m.radius) {
            float t = 1.0f - (dist / m.radius); // 1 at center -> 0 at edge
            y += m.height * (t * t);            // quadratic falloff
        }
    }
    return y;
}

void drawTerrain() {
    const int SIZE = 64;        // Grid size (64x64)
    const float SPACING = 0.5f; // Distance between vertices

    glShadeModel(GL_SMOOTH);
    glBegin(GL_QUADS);
    for (int i = -SIZE/2; i < SIZE/2; ++i) {
        for (int j = -SIZE/2; j < SIZE/2; ++j) {
            float x1 = i * SPACING;
            float z1 = j * SPACING;
            float x2 = (i + 1) * SPACING;
            float z2 = (j + 1) * SPACING;

            float y11 = getTerrainHeight(x1, z1);
            float y12 = getTerrainHeight(x1, z2);
            float y21 = getTerrainHeight(x2, z1);
            float y22 = getTerrainHeight(x2, z2);

            // Simple color variation by average height
            float avg = (y11 + y12 + y21 + y22) * 0.25f;
            float color = std::clamp(0.2f + 0.3f + avg * 0.2f, 0.1f, 0.9f);
            glColor3f(0.0f, color, 0.0f);

            glVertex3f(x1, y11, z1);
            glVertex3f(x2, y21, z1);
            glVertex3f(x2, y22, z2);
            glVertex3f(x1, y12, z2);
        }
    }
    glEnd();
}