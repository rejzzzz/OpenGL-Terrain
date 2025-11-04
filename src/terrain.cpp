#include <GL/glew.h>
#include <cmath>
#include <vector>
#include <algorithm>
#include <glm/glm.hpp>
// Allow terrain to consult pond definitions so we can carve basins
#include "../include/objects.h"
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

// Base rolling hills + optional mountain domes (no pond deformation)
float getTerrainBaseHeight(float x, float z) {
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

// Final terrain height including pond basins carved out of the base terrain
float getTerrainHeight(float x, float z) {
    float y = getTerrainBaseHeight(x, z);
    // Carve pond basins so water sits in a depression instead of following slopes
    const auto &ponds = getPonds();
    for (const auto &pp : ponds) {
        const glm::vec2 &c = pp.first;
        float r = pp.second;
        float dx = x - c.x;
        float dz = z - c.y;
        float dist = std::sqrt(dx*dx + dz*dz);
        if (dist < r) {
            // compute a center base height and a max depth proportional to radius
            float centerBase = getTerrainBaseHeight(c.x, c.y);
            float maxDepth = std::min(3.0f, r * 0.35f);
            float falloff = 1.0f - (dist / r);
            float depress = maxDepth * (falloff * falloff);
            float depressedY = centerBase - depress;
            if (y > depressedY) y = depressedY;
        }
    }
    return y;
}

// Return strongest mountain influence [0..1] at (x,z)
static float getMountainContribution(float x, float z) {
    float best = 0.0f;
    for (const auto& m : s_mountains) {
        float dx = x - m.center.x;
        float dz = z - m.center.y;
        float dist = std::sqrt(dx*dx + dz*dz);
        if (dist < m.radius) {
            float t = 1.0f - (dist / m.radius);
            if (t > best) best = t; // keep max influence
        }
    }
    return best;
}

void drawTerrain() {
    // Grid size (SIZE x SIZE) and spacing between vertices
    const int SIZE = 200;        // Grid size (200x200)
    const float SPACING = 0.5f;  // Distance between vertices

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

            // Simple color variation by average height and mountain contribution
            float avg = (y11 + y12 + y21 + y22) * 0.25f;
            // Compute mountain influence per-vertex and take the max for the quad
            float m11 = getMountainContribution(x1, z1);
            float m12 = getMountainContribution(x1, z2);
            float m21 = getMountainContribution(x2, z1);
            float m22 = getMountainContribution(x2, z2);
            float m = std::max(std::max(m11, m12), std::max(m21, m22));

            // Colors
            const float greenR = 0.05f, greenG = 0.45f, greenB = 0.05f;
            const float brownR = 0.45f, brownG = 0.30f, brownB = 0.18f;
            const float peakR = 0.95f, peakG = 0.95f, peakB = 0.95f;

            float r,g,b;
            if (m > 0.0f) {
                // Mountain area: fully brown body (no green blending)
                float mixMB = std::clamp(m, 0.0f, 1.0f);
                r = brownR;
                g = brownG;
                b = brownB;

                // Peak: blend toward white for strongest influence
                if (m > 0.65f) {
                    float tpeak = (m - 0.65f) / (1.0f - 0.65f); // 0..1
                    r = r * (1.0f - tpeak) + peakR * tpeak;
                    g = g * (1.0f - tpeak) + peakG * tpeak;
                    b = b * (1.0f - tpeak) + peakB * tpeak;
                }
            } else {
                // Non-mountain grassy color varied by height
                float color = std::clamp(0.35f + avg * 0.1f, 0.05f, 0.9f);
                r = greenR * color;
                g = greenG * color;
                b = greenB * color;
            }
            glColor3f(r, g, b);

            glVertex3f(x1, y11, z1);
            glVertex3f(x2, y21, z1);
            glVertex3f(x2, y22, z2);
            glVertex3f(x1, y12, z2);
        }
    }
    glEnd();
}