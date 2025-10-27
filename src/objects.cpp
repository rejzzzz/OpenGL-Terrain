#include <GL/glew.h>
#include "../include/terrain.h"
#include <vector>
#include <cmath>

// Helper: draw a simple axis-aligned cube centered at origin
void drawCube(float width, float height, float depth) {
    float w2 = width * 0.5f;
    float h2 = height * 0.5f;
    float d2 = depth * 0.5f;

    glBegin(GL_QUADS);
    // Front face
    glColor3f(0.6f, 0.6f, 0.6f);
    glVertex3f(-w2, -h2,  d2);
    glVertex3f( w2, -h2,  d2);
    glVertex3f( w2,  h2,  d2);
    glVertex3f(-w2,  h2,  d2);
    // Back face
    glVertex3f(-w2, -h2, -d2);
    glVertex3f(-w2,  h2, -d2);
    glVertex3f( w2,  h2, -d2);
    glVertex3f( w2, -h2, -d2);
    // Top face
    glColor3f(0.7f, 0.7f, 0.7f);
    glVertex3f(-w2,  h2, -d2);
    glVertex3f(-w2,  h2,  d2);
    glVertex3f( w2,  h2,  d2);
    glVertex3f( w2,  h2, -d2);
    // Bottom face
    glColor3f(0.5f, 0.5f, 0.5f);
    glVertex3f(-w2, -h2, -d2);
    glVertex3f( w2, -h2, -d2);
    glVertex3f( w2, -h2,  d2);
    glVertex3f(-w2, -h2,  d2);
    // Right face
    glColor3f(0.65f, 0.65f, 0.65f);
    glVertex3f( w2, -h2, -d2);
    glVertex3f( w2,  h2, -d2);
    glVertex3f( w2,  h2,  d2);
    glVertex3f( w2, -h2,  d2);
    // Left face
    glVertex3f(-w2, -h2, -d2);
    glVertex3f(-w2, -h2,  d2);
    glVertex3f(-w2,  h2,  d2);
    glVertex3f(-w2,  h2, -d2);
    glEnd();
}

// Draw a simple tree using cubes only (no GLUT)
void drawTree(float x, float z) {
    float y = getTerrainHeight(x, z);
    glPushMatrix();
    glTranslatef(x, y, z);

    // Trunk: small brown cube
    glColor3f(0.35f, 0.2f, 0.1f); // Brown
    drawCube(0.3f, 1.0f, 0.3f);

    // Foliage: green cube on top (crude, but works)
    glTranslatef(0.0f, 1.0f, 0.0f); // Move up
    glColor3f(0.0f, 0.7f, 0.2f);   // Green
    drawCube(1.2f, 1.2f, 1.2f);

    glPopMatrix();
}

void drawTrees() {
    drawTree(5.0f, 5.0f);
    drawTree(-6.0f, 3.0f);
    drawTree(2.0f, -7.0f);
}

void drawBuildings() {
    // Building 1
    float x1 = -4.0f, z1 = -4.0f;
    float y1 = getTerrainHeight(x1, z1);
    glPushMatrix();
    glTranslatef(x1, y1, z1);
    drawCube(2.0f, 3.0f, 2.0f); // building
    // Add simple windows on the front face (facing +Z)
    // Building dimensions
    float bw = 2.0f; float bh = 3.0f; float bd = 2.0f;
    // Front face is at +d2 (since drawCube was centered)
    float w2 = bw * 0.5f;
    float h2 = bh * 0.5f;
    float d2 = bd * 0.5f;

    // Draw a grid of windows: 2 columns x 3 rows
    glColor3f(0.95f, 0.95f, 0.6f); // warm window light
    for (int row = 0; row < 3; ++row) {
        for (int col = 0; col < 2; ++col) {
            float wx = -w2 + 0.5f + col * 0.9f; // local x
            float wy = -h2 + 0.6f + row * 0.9f; // local y
            float ww = 0.35f, wh = 0.4f;
            glBegin(GL_QUADS);
            // window quad placed slightly in front of face to avoid z-fighting
            float zoff = d2 + 0.001f;
            glVertex3f(wx - ww*0.5f, wy - wh*0.5f,  zoff);
            glVertex3f(wx + ww*0.5f, wy - wh*0.5f,  zoff);
            glVertex3f(wx + ww*0.5f, wy + wh*0.5f,  zoff);
            glVertex3f(wx - ww*0.5f, wy + wh*0.5f,  zoff);
            glEnd();
        }
    }
    glPopMatrix();

    // Additional buildings: create a small cluster of 4 extra buildings
    struct Bld { float x,z,w,h,d; } extra[] = {
        { 8.5f,  6.5f, 1.6f, 2.0f, 1.6f },
        { -6.0f, -2.0f, 2.2f, 3.2f, 2.0f },
        { 3.0f, -9.0f,  1.4f, 1.8f, 1.4f },
        { -10.0f, 8.0f, 2.5f, 3.5f, 2.2f }
    };
    for (const auto &b : extra) {
        float bx = b.x, bz = b.z;
        float by = getTerrainHeight(bx, bz);
        glPushMatrix();
        glTranslatef(bx, by, bz);
        drawCube(b.w, b.h, b.d);

        // windows: attempt 2 columns x rows depending on height
        float bwid = b.w, bht = b.h, bdep = b.d;
        float halfW = bwid * 0.5f; float halfH = bht * 0.5f; float halfD = bdep * 0.5f;
        int rows = (int)std::max(1.0f, std::floor(bht));
        int cols = 2;
        glColor3f(0.95f, 0.9f, 0.55f);
        for (int row = 0; row < rows; ++row) {
            for (int col = 0; col < cols; ++col) {
                float wx = -halfW + 0.6f + col * (bwid - 1.2f);
                float wy = -halfH + 0.6f + row * 0.9f;
                float ww = 0.35f, wh = 0.4f;
                glBegin(GL_QUADS);
                float zoff = halfD + 0.001f;
                glVertex3f(wx - ww*0.5f, wy - wh*0.5f,  zoff);
                glVertex3f(wx + ww*0.5f, wy - wh*0.5f,  zoff);
                glVertex3f(wx + ww*0.5f, wy + wh*0.5f,  zoff);
                glVertex3f(wx - ww*0.5f, wy + wh*0.5f,  zoff);
                glEnd();
            }
        }
        glPopMatrix();
    }
}

// Draw a road following a polyline of waypoints on the terrain.
void drawRoads() {
    // Simple waypoint path connecting building cluster and main building
    struct P { float x,z; };
    std::vector<P> waypoints = {
        { -4.0f, -4.0f }, // building 1
        { 0.0f, 0.0f },
        { 4.0f, 2.0f },
        { 8.5f, 6.5f }   // extra building cluster
    };

    // Sample along the polyline
    std::vector<glm::vec3> samples;
    const float sampleSpacing = 0.5f;
    for (size_t i = 0; i + 1 < waypoints.size(); ++i) {
        float x0 = waypoints[i].x, z0 = waypoints[i].z;
        float x1 = waypoints[i+1].x, z1 = waypoints[i+1].z;
        float dx = x1 - x0, dz = z1 - z0;
        float segLen = std::sqrt(dx*dx + dz*dz);
        int steps = std::max(1, (int)std::ceil(segLen / sampleSpacing));
        for (int s = 0; s <= steps; ++s) {
            float t = (float)s / (float)steps;
            float x = x0 + dx * t;
            float z = z0 + dz * t;
            float y = getTerrainHeight(x, z) + 0.02f; // slight offset to avoid z-fighting
            samples.push_back(glm::vec3(x, y, z));
        }
    }

    if (samples.size() < 2) return;

    // Road width
    const float roadWidth = 1.2f;

    // Draw road as a triangle strip using a perp to the tangent
    glColor3f(0.18f, 0.18f, 0.18f); // dark asphalt
    glBegin(GL_TRIANGLE_STRIP);
    for (size_t i = 0; i < samples.size(); ++i) {
        // tangent
        glm::vec3 prev = (i == 0) ? samples[i] : samples[i-1];
        glm::vec3 next = (i + 1 == samples.size()) ? samples[i] : samples[i+1];
        glm::vec3 tangent = glm::normalize(next - prev);
        // perpendicular in XZ plane
        glm::vec3 perp(-tangent.z, 0.0f, tangent.x);
        // left and right vertices
        glm::vec3 left = samples[i] + perp * (roadWidth * 0.5f);
        glm::vec3 right = samples[i] - perp * (roadWidth * 0.5f);
        glVertex3f(left.x, left.y, left.z);
        glVertex3f(right.x, right.y, right.z);
    }
    glEnd();

    // Optional: draw center line
    glColor3f(0.95f, 0.9f, 0.4f);
    glLineWidth(2.0f);
    glBegin(GL_LINE_STRIP);
    for (size_t i = 0; i < samples.size(); ++i) {
        glVertex3f(samples[i].x, samples[i].y + 0.015f, samples[i].z);
    }
    glEnd();
    glLineWidth(1.0f);
}