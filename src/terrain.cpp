#include <GL/glew.h>
#include <cmath>

// Simple function to get height at (x, z)
float getTerrainHeight(float x, float z) {
    // Create gentle rolling hills with reduced amplitude
    return 0.5f * sin(x * 0.2f) * cos(z * 0.2f);
}

void drawTerrain() {
    const int SIZE = 64;        // Grid size (64x64)
    const float SPACING = 0.5f; // Distance between vertices

    // Enable smooth shading
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

            // Color based on height
            float color = 0.2f + 0.6f * ((y11 + y12 + y21 + y22) / 4.0f + 2.0f) / 4.0f;
            glColor3f(0.0f, color, 0.0f); // Greenish

            // Draw quad (two triangles would be better, but quads work here)
            glVertex3f(x1, y11, z1);
            glVertex3f(x2, y21, z1);
            glVertex3f(x2, y22, z2);
            glVertex3f(x1, y12, z2);
        }
    }
    glEnd();
}