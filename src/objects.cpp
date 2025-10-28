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

// forward declaration for helper defined later
static void drawAllBuildings();

void drawBuildings() {
    // Draw all buildings using the centralized helper
    drawAllBuildings();
}

// Helper: draw a building at world x,z with width, height, depth and window color
static void drawBuildingAt(float wx, float wz, float bw, float bh, float bd, const glm::vec3 &windowColor) {
    float wy = getTerrainHeight(wx, wz);
    glPushMatrix();
    glTranslatef(wx, wy, wz);

    // building body
    glColor3f(0.6f, 0.6f, 0.6f);
    drawCube(bw, bh, bd);

    float halfW = bw * 0.5f;
    float halfH = bh * 0.5f;
    float halfD = bd * 0.5f;

    // window grid parameters
    int rows = std::max(1, (int)std::floor(bh));
    int cols = 2;
    float ww = std::min(0.5f, bw * 0.25f);
    float wh = std::min(0.6f, bh * 0.18f);

    glColor3f(windowColor.r, windowColor.g, windowColor.b);

    // Front (+Z) and Back (-Z)
    for (int row = 0; row < rows; ++row) {
        for (int col = 0; col < cols; ++col) {
            float lx = -halfW + 0.6f + col * (bw - 1.2f);
            float ly = -halfH + 0.6f + row * 0.9f;
            float zoff = halfD + 0.002f;
            // front
            glBegin(GL_QUADS);
            glVertex3f(lx - ww*0.5f, ly - wh*0.5f,  zoff);
            glVertex3f(lx + ww*0.5f, ly - wh*0.5f,  zoff);
            glVertex3f(lx + ww*0.5f, ly + wh*0.5f,  zoff);
            glVertex3f(lx - ww*0.5f, ly + wh*0.5f,  zoff);
            glEnd();

            // back (mirror x)
            glBegin(GL_QUADS);
            glVertex3f(-lx - ww*0.5f, ly - wh*0.5f, -zoff);
            glVertex3f(-lx + ww*0.5f, ly - wh*0.5f, -zoff);
            glVertex3f(-lx + ww*0.5f, ly + wh*0.5f, -zoff);
            glVertex3f(-lx - ww*0.5f, ly + wh*0.5f, -zoff);
            glEnd();
        }
    }

    // Left (-X) and Right (+X)
    for (int row = 0; row < rows; ++row) {
        for (int col = 0; col < cols; ++col) {
            float lz = -halfD + 0.6f + col * (bd - 1.2f);
            float ly = -halfH + 0.6f + row * 0.9f;
            float xoff = halfW + 0.002f;
            // right (+X)
            glBegin(GL_QUADS);
            glVertex3f( xoff, ly - wh*0.5f, lz - ww*0.5f);
            glVertex3f( xoff, ly - wh*0.5f, lz + ww*0.5f);
            glVertex3f( xoff, ly + wh*0.5f, lz + ww*0.5f);
            glVertex3f( xoff, ly + wh*0.5f, lz - ww*0.5f);
            glEnd();

            // left (-X)
            glBegin(GL_QUADS);
            glVertex3f(-xoff, ly - wh*0.5f, -lz - ww*0.5f);
            glVertex3f(-xoff, ly - wh*0.5f, -lz + ww*0.5f);
            glVertex3f(-xoff, ly + wh*0.5f, -lz + ww*0.5f);
            glVertex3f(-xoff, ly + wh*0.5f, -lz - ww*0.5f);
            glEnd();
        }
    }

    glPopMatrix();
}

// Replace previous ad-hoc draws with calls to drawBuildingAt
// We'll draw the main buildings and extra cluster
static void drawAllBuildings() {
    // Main building 1
    drawBuildingAt(-4.0f, -4.0f, 2.0f, 3.0f, 2.0f, glm::vec3(0.95f,0.95f,0.6f));
    // Building 2
    drawBuildingAt(6.0f, 4.0f, 1.8f, 2.5f, 1.8f, glm::vec3(0.9f,0.9f,0.5f));
    // Extra cluster
    drawBuildingAt(8.5f,  6.5f, 1.6f, 2.0f, 1.6f, glm::vec3(0.95f,0.9f,0.55f));
    drawBuildingAt(-6.0f, -2.0f, 2.2f, 3.2f, 2.0f, glm::vec3(0.95f,0.9f,0.55f));
    drawBuildingAt(3.0f, -9.0f,  1.4f, 1.8f, 1.4f, glm::vec3(0.95f,0.9f,0.55f));
    drawBuildingAt(-10.0f, 8.0f, 2.5f, 3.5f, 2.2f, glm::vec3(0.95f,0.9f,0.55f));
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
    // center dashed line: draw small quads every N samples
    glColor3f(0.95f, 0.9f, 0.4f);
    const float dashLen = 0.6f;
    const float dashGap = 0.4f;
    float acc = 0.0f;
    for (size_t i = 1; i < samples.size(); ++i) {
        glm::vec3 a = samples[i-1];
        glm::vec3 b = samples[i];
        float seg = glm::length(b - a);
        glm::vec3 dir = glm::normalize(b - a);
        float t = 0.0f;
        while (t < seg) {
            float avail = seg - t;
            float take = std::min(dashLen, avail);
            if (acc <= 0.0001f) {
                // draw dash from a + dir*(t) to a + dir*(t+take)
                glm::vec3 p0 = a + dir * t;
                glm::vec3 p1 = a + dir * (t + take);
                // small quad perpendicular to dir
                glm::vec3 perp(-dir.z, 0.0f, dir.x);
                float half = 0.06f;
                glBegin(GL_QUADS);
                glVertex3f(p0.x - perp.x*half, p0.y + 0.02f, p0.z - perp.z*half);
                glVertex3f(p0.x + perp.x*half, p0.y + 0.02f, p0.z + perp.z*half);
                glVertex3f(p1.x + perp.x*half, p1.y + 0.02f, p1.z + perp.z*half);
                glVertex3f(p1.x - perp.x*half, p1.y + 0.02f, p1.z - perp.z*half);
                glEnd();
                acc = dashGap;
            } else {
                acc -= take;
            }
            t += take;
        }
    }
}