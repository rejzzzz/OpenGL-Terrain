// Clean implementation for buildings, roads and simple trees.

#include <GL/glew.h>
#include <glm/glm.hpp>
#include <random>
#include <GLFW/glfw3.h>
#include <vector>
#include <cmath>
#include <algorithm>
#include <string>
#include "stb_image.h"

#include "../include/terrain.h"
#include "../include/objects.h"

// Static texture handles for buildings (0=none, 1=brick, 2=metal)
static GLuint g_buildingTextures[3] = {0, 0, 0};

bool isPositionInsideBuilding(float x, float z, float radius);

// Simple texture loader for building diffuse
void initBuildingTexture(const std::string &path, int textureType) {
    if (textureType < 0 || textureType > 2) return;
    
    if (g_buildingTextures[textureType]) {
        glDeleteTextures(1, &g_buildingTextures[textureType]);
        g_buildingTextures[textureType] = 0;
    }

    int width, height, nrChannels;
    unsigned char *data = stbi_load(path.c_str(), &width, &height, &nrChannels, 0);
    if (!data) {
        printf("Failed to load building texture: %s\n", path.c_str());
        return;
    }

    GLenum format = (nrChannels == 4) ? GL_RGBA : GL_RGB;
    glGenTextures(1, &g_buildingTextures[textureType]);
    glBindTexture(GL_TEXTURE_2D, g_buildingTextures[textureType]);
    glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glBindTexture(GL_TEXTURE_2D, 0);
    stbi_image_free(data);
    printf("Loaded building texture type %d: %s (%dx%d, %d channels)\n", textureType, path.c_str(), width, height, nrChannels);
}

void initBuildingTextures(const std::string &brickPath, const std::string &metalPath) {
    initBuildingTexture(brickPath, 1);
    initBuildingTexture(metalPath, 2);
}

static void drawCube(float width, float height, float depth, bool shaded = false) {
    float w2 = width * 0.5f;
    float h2 = height * 0.5f;
    float d2 = depth * 0.5f;
    glBegin(GL_QUADS);
    // front
    glTexCoord2f(0.0f, 0.0f); glVertex3f(-w2, -h2,  d2);
    glTexCoord2f(1.0f, 0.0f); glVertex3f( w2, -h2,  d2);
    glTexCoord2f(1.0f, 1.0f); glVertex3f( w2,  h2,  d2);
    glTexCoord2f(0.0f, 1.0f); glVertex3f(-w2,  h2,  d2);
    // back
    glTexCoord2f(1.0f, 0.0f); glVertex3f(-w2, -h2, -d2);
    glTexCoord2f(1.0f, 1.0f); glVertex3f(-w2,  h2, -d2);
    glTexCoord2f(0.0f, 1.0f); glVertex3f( w2,  h2, -d2);
    glTexCoord2f(0.0f, 0.0f); glVertex3f( w2, -h2, -d2);
    // top
    glTexCoord2f(0.0f, 1.0f); glVertex3f(-w2,  h2, -d2);
    glTexCoord2f(0.0f, 0.0f); glVertex3f(-w2,  h2,  d2);
    glTexCoord2f(1.0f, 0.0f); glVertex3f( w2,  h2,  d2);
    glTexCoord2f(1.0f, 1.0f); glVertex3f( w2,  h2, -d2);
    // bottom
    glTexCoord2f(1.0f, 1.0f); glVertex3f(-w2, -h2, -d2);
    glTexCoord2f(0.0f, 1.0f); glVertex3f( w2, -h2, -d2);
    glTexCoord2f(0.0f, 0.0f); glVertex3f( w2, -h2,  d2);
    glTexCoord2f(1.0f, 0.0f); glVertex3f(-w2, -h2,  d2);
    // right
    glTexCoord2f(0.0f, 0.0f); glVertex3f( w2, -h2, -d2);
    glTexCoord2f(1.0f, 0.0f); glVertex3f( w2,  h2, -d2);
    glTexCoord2f(1.0f, 1.0f); glVertex3f( w2,  h2,  d2);
    glTexCoord2f(0.0f, 1.0f); glVertex3f( w2, -h2,  d2);
    // left
    glTexCoord2f(0.0f, 0.0f); glVertex3f(-w2, -h2, -d2);
    glTexCoord2f(1.0f, 0.0f); glVertex3f(-w2, -h2,  d2);
    glTexCoord2f(1.0f, 1.0f); glVertex3f(-w2,  h2,  d2);
    glTexCoord2f(0.0f, 1.0f); glVertex3f(-w2,  h2, -d2);
    glEnd();
}

static void drawTree(float x, float z) {
    float y = getTerrainHeight(x, z);
    glPushMatrix();
    glTranslatef(x, y + 0.05f, z);
    glColor3f(0.4f, 0.25f, 0.1f);
    drawCube(0.25f, 0.8f, 0.25f, false);
    glTranslatef(0.0f, 0.8f, 0.0f);
    glColor3f(0.1f, 0.6f, 0.1f);
    drawCube(1.0f, 1.0f, 1.0f, false);
    glPopMatrix();
}

// helper: filled circular cap (triangle fan) to cover intersections
// Draw a filled disk (triangle fan) positioned just above the local terrain to avoid being
// occluded by nearby triangles. Use polygon offset when drawing water to reduce z-fighting.
static void drawFilledDisk(float cx, float cz, float radius, int segments = 20) {
    // place the water a hair above the terrain height so it can be blended on top
    float centerY = getTerrainHeight(cx, cz) + 0.001f;
    glBegin(GL_TRIANGLE_FAN);
    glVertex3f(cx, centerY, cz);
    for (int i = 0; i <= segments; ++i) {
        float a = (float)i / (float)segments * 2.0f * 3.14159265f;
        float x = cx + std::cos(a) * radius;
        float z = cz + std::sin(a) * radius;
        float yy = getTerrainHeight(x, z) + 0.001f;
        glVertex3f(x, yy, z);
    }
    glEnd();
}

// Draw an annulus (ring) between innerRadius and outerRadius. This is used for the
// shore/side band so we don't cover the full water disk with a larger filled disk.
static void drawDiskAnnulus(float cx, float cz, float innerRadius, float outerRadius, int segments = 24) {
    float centerY = getTerrainHeight(cx, cz) + 0.001f;
    glBegin(GL_TRIANGLE_STRIP);
    for (int i = 0; i <= segments; ++i) {
        float a = (float)i / (float)segments * 2.0f * 3.14159265f;
        float ix = cx + std::cos(a) * innerRadius;
        float iz = cz + std::sin(a) * innerRadius;
        float ox = cx + std::cos(a) * outerRadius;
        float oz = cz + std::sin(a) * outerRadius;
        float iy = getTerrainHeight(ix, iz) + 0.001f;
        float oy = getTerrainHeight(ox, oz) + 0.001f;
        glVertex3f(ix, iy, iz);
        glVertex3f(ox, oy, oz);
    }
    glEnd();
}

// Draw water surface with depth gradient (darker in center, lighter at edges)
static void drawWaterSurface(float cx, float cz, float radius, float baseY, int segments = 64) {
    // Create a bowl-shaped depression for more realistic water
    const float depthFactor = 0.8f;  // How deep the center dips
    
    glBegin(GL_TRIANGLE_FAN);
    // Center vertex - deepest, darkest blue
    glColor4f(0.02f, 0.25f, 0.6f, 0.85f);
    glVertex3f(cx, baseY - depthFactor, cz);
    
    // Outer ring - lighter blue, shallower
    for (int i = 0; i <= segments; ++i) {
        float a = (float)i / (float)segments * 2.0f * 3.14159265f;
        float x = cx + std::cos(a) * radius;
        float z = cz + std::sin(a) * radius;
        // Lighter color at edges
        glColor4f(0.1f, 0.5f, 0.85f, 0.7f);
        glVertex3f(x, baseY - depthFactor * 0.2f, z);
    }
    glEnd();
    
    // Add subtle ripple rings for visual interest
    glLineWidth(1.0f);
    for (int ring = 1; ring <= 3; ++ring) {
        float rippleRadius = radius * (0.3f + ring * 0.2f);
        if (rippleRadius >= radius) break;
        float rippleY = baseY - depthFactor * (1.0f - rippleRadius / radius);
        
        glColor4f(0.15f, 0.6f, 0.9f, 0.3f);
        glBegin(GL_LINE_LOOP);
        for (int i = 0; i < segments; ++i) {
            float a = (float)i / (float)segments * 2.0f * 3.14159265f;
            float x = cx + std::cos(a) * rippleRadius;
            float z = cz + std::sin(a) * rippleRadius;
            glVertex3f(x, rippleY, z);
        }
        glEnd();
    }
    glLineWidth(1.0f);
}

// helper: 2D point-segment distance
static float pointSegDist2D(const glm::vec2 &p, const glm::vec2 &a, const glm::vec2 &b) {
    glm::vec2 v = b - a;
    glm::vec2 w = p - a;
    float c1 = w.x * v.x + w.y * v.y;
    if (c1 <= 0.0f) return std::sqrt((p.x - a.x)*(p.x - a.x) + (p.y - a.y)*(p.y - a.y));
    float c2 = v.x * v.x + v.y * v.y;
    if (c2 <= 0.0f) return std::sqrt((p.x - a.x)*(p.x - a.x) + (p.y - a.y)*(p.y - a.y));
    float t = c1 / c2;
    if (t < 0.0f) t = 0.0f; else if (t > 1.0f) t = 1.0f;
    glm::vec2 proj = a + v * t;
    return std::sqrt((p.x - proj.x)*(p.x - proj.x) + (p.y - proj.y)*(p.y - proj.y));
}

// Ponds storage
static std::vector<std::pair<glm::vec2,float>> s_ponds;

void addPond(const glm::vec2 &center, float radius) { s_ponds.emplace_back(center, radius); }
void clearPonds() { s_ponds.clear(); }
const std::vector<std::pair<glm::vec2,float>>& getPonds() { return s_ponds; }



void drawPonds() {
    for (const auto &pp : s_ponds) {
        const glm::vec2 &c = pp.first;
        float r = pp.second;
        
        // Determine base water level at pond center
        float centerHeight = getTerrainHeight(c.x, c.y);
        float waterY = centerHeight + 3.5f;  // Raised higher for better visibility
        
        // Draw sandy/muddy shore band (wider and more natural looking)
        glColor3f(0.72f, 0.6f, 0.42f);  // Sandy beach color
        drawDiskAnnulus(c.x, c.y, r * 0.95f, r + 1.2f, 32);
        
        // Draw darker wet sand/mud transition
        glColor3f(0.45f, 0.35f, 0.25f);  // Wet sand
        drawDiskAnnulus(c.x, c.y, r * 0.92f, r * 0.95f, 24);
        
        // Draw water surface with depth and transparency
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glDepthMask(GL_FALSE);  // Don't write to depth buffer for transparent water
        
        drawWaterSurface(c.x, c.y, r * 0.92f, waterY, 64);
        
        glDepthMask(GL_TRUE);
        glDisable(GL_BLEND);
    }
}

// Street lights storage
static std::vector<glm::vec3> s_streetLights;
void addStreetLight(const glm::vec3 &pos) { s_streetLights.push_back(pos); }
void clearStreetLights() { s_streetLights.clear(); }

void drawStreetLights() {
    for (const auto &p : s_streetLights) {
        float y = getTerrainHeight(p.x, p.z);
        glPushMatrix();
        glTranslatef(p.x, y, p.z);
        // pole
        glColor3f(0.15f, 0.15f, 0.15f);
        glPushMatrix();
        glTranslatef(0.0f, 2.0f, 0.0f);
        glScalef(0.08f, 4.0f, 0.08f);
        drawCube(1.0f, 1.0f, 1.0f, false);
        glPopMatrix();
        // lamp head
        glPushMatrix();
        glTranslatef(0.0f, 4.05f, 0.25f);
        glScalef(0.35f, 0.18f, 0.2f);
        glColor3f(0.95f, 0.95f, 0.8f);
        drawCube(1.0f, 1.0f, 1.0f, false);
        glPopMatrix();
        // small light glow (simple quad)
        glPushMatrix();
        glTranslatef(0.0f, 4.2f, 0.42f);
        glColor3f(1.0f, 0.95f, 0.8f);
        glBegin(GL_QUADS);
        glVertex3f(-0.08f, -0.02f, 0.0f);
        glVertex3f( 0.08f, -0.02f, 0.0f);
        glVertex3f( 0.08f,  0.02f, 0.0f);
        glVertex3f(-0.08f,  0.02f, 0.0f);
        glEnd();
        glPopMatrix();
        glPopMatrix();
    }
}

// Storage for buildings and roads
static std::vector<BuildingDef> s_buildings;
static std::vector<Road> s_roads;
static std::vector<glm::vec2> s_trees;

static void ensureTreesInitialized();

void drawTrees() {
    ensureTreesInitialized();
    for (const auto &t : s_trees) {
        drawTree(t.x, t.y);
    }
}

static void ensureTreesInitialized() {
    if (!s_trees.empty()) return;
    // Only add trees that are far from the lake at (-25, 25)
    glm::vec2 lakePos(-25.0f, 25.0f);
    float lakeRad = 10.0f;
    
    std::vector<glm::vec2> candidates = {{5.0f, 5.0f}, {-6.0f, 3.0f}, {2.0f, -7.0f}};
    for (const auto& pos : candidates) {
        float distToLake = std::sqrt((pos.x - lakePos.x)*(pos.x - lakePos.x) + (pos.y - lakePos.y)*(pos.y - lakePos.y));
        if (distToLake > lakeRad + 5.0f) {
            s_trees.push_back(pos);
        }
    }
}

// Coins storage (defined here where s_roads is visible)
struct Coin { glm::vec2 p; bool collected; };
static std::vector<Coin> s_coins;

void clearCoins() { s_coins.clear(); }
int getCollectedCoinsCount() { int c=0; for (auto &co : s_coins) if (co.collected) ++c; return c; }
int getTotalCoinsCount() { return (int)s_coins.size(); }

// spawn N coins randomly within a circle of radius areaRadius centered at origin
void spawnCoins(int n, float areaRadius) {
    // Place a mix of coins on roads and beside buildings to feel like game collectibles.
    s_coins.clear();
    std::mt19937 rng(1234567);
    std::uniform_real_distribution<float> unit(0.0f, 1.0f);

    int placed = 0;
    int attempts = 0;
    const int maxAttempts = n * 50 + 500;
    while (placed < n && attempts < maxAttempts) {
        ++attempts;
        // Decide placement type: 40% on roads, 60% near buildings (if available)
        float pick = unit(rng);
        float x=0.0f, z=0.0f;
        bool ok = false;
        if (pick < 0.4f && !s_roads.empty()) {
            // pick a random road and a random position along it
            std::uniform_int_distribution<int> rindex(0, (int)s_roads.size()-1);
            int ri = rindex(rng);
            const Road &rd = s_roads[ri];
            if (rd.pts.size() < 2) continue;
            // pick random segment proportional to length
            std::vector<float> segLen(rd.pts.size()-1);
            float total=0.0f;
            for (size_t i=0;i+1<rd.pts.size();++i){
                float dx = rd.pts[i+1].x - rd.pts[i].x;
                float dz = rd.pts[i+1].y - rd.pts[i].y;
                float L = std::sqrt(dx*dx + dz*dz);
                segLen[i]=L; total+=L;
            }
            if (total <= 0.0f) continue;
            float t = unit(rng) * total;
            size_t seg = 0; float acc=0.0f;
            for (; seg < segLen.size(); ++seg) { if (acc + segLen[seg] >= t) break; acc += segLen[seg]; }
            if (seg >= segLen.size()) seg = segLen.size()-1;
            float localT = (t - acc) / segLen[seg];
            glm::vec2 a = rd.pts[seg]; glm::vec2 b = rd.pts[seg+1];
            x = a.x + (b.x - a.x) * localT;
            z = a.y + (b.y - a.y) * localT;
            // offset slightly to sit on road/sidewalk: choose either center or sidewalk
            float sideOff = (unit(rng) < 0.5f) ? 0.0f : (rd.halfWidth + 0.25f);
            // compute tangent
            glm::vec2 dir = b - a; float dlen = std::sqrt(dir.x*dir.x + dir.y*dir.y);
            if (dlen < 1e-5f) continue;
            dir /= dlen;
            glm::vec2 perp(-dir.y, dir.x);
            float side = (unit(rng) < 0.5f) ? 1.0f : -1.0f;
            x += perp.x * (sideOff * side);
            z += perp.y * (sideOff * side);
            ok = true;
        } else if (!s_buildings.empty()) {
            // pick a random building and place coin near one of its sides
            std::uniform_int_distribution<int> bindex(0, (int)s_buildings.size()-1);
            int bi = bindex(rng);
            const BuildingDef &b = s_buildings[bi];
            // choose a side (0..3) and an offset along that side
            int side = (int)(unit(rng) * 4.0f);
            float off = (unit(rng) * (std::max(b.bw, b.bd) - 0.2f)) - (std::max(b.bw, b.bd)/2.0f - 0.1f);
            if (side == 0) { // +x side
                x = b.x + b.bw*0.5f + 0.35f; z = b.z + off;
            } else if (side == 1) { // -x
                x = b.x - b.bw*0.5f - 0.35f; z = b.z + off;
            } else if (side == 2) { // +z
                z = b.z + b.bd*0.5f + 0.35f; x = b.x + off;
            } else {
                z = b.z - b.bd*0.5f - 0.35f; x = b.x + off;
            }
            // small jitter
            x += (unit(rng) - 0.5f) * 0.25f;
            z += (unit(rng) - 0.5f) * 0.25f;
            // ensure not inside building
            if (isPositionInsideBuilding(x, z, 0.2f)) continue;
            ok = true;
        }
        if (!ok) continue;
        // final sanity: ensure within areaRadius
        if (std::sqrt(x*x + z*z) > areaRadius * 1.05f) continue;
        
        // Avoid placing coins in the lake area
        glm::vec2 lakePos(-25.0f, 25.0f);
        float lakeRad = 10.0f;
        float distToLake = std::sqrt((x - lakePos.x)*(x - lakePos.x) + (z - lakePos.y)*(z - lakePos.y));
        if (distToLake < lakeRad + 5.0f) continue;
        
        s_coins.push_back(Coin{glm::vec2(x,z), false});
        ++placed;
    }
}

// draw coins as small gold boxes sitting on the terrain
void drawCoins() {
    glColor3f(0.95f, 0.8f, 0.1f);
    double t = 0.0;
    // use glfwGetTime if available to animate bobbing
#ifdef GLFW_INCLUDE_NONE
    t = 0.0;
#else
    t = glfwGetTime();
#endif
    // helper: draw a thin vertical coin (circular faces in Y-Z plane, thickness along X)
    auto drawVerticalCoin = [](float radius, float halfThickness, int segments = 16) {  // Reduced from 32 to 16 for performance
        // front face (x = +halfThickness)
        glBegin(GL_TRIANGLE_FAN);
        glVertex3f(halfThickness, 0.0f, 0.0f);
        for (int i = 0; i <= segments; ++i) {
            float a = (float)i / (float)segments * 2.0f * 3.14159265f;
            float y = std::cos(a) * radius;
            float z = std::sin(a) * radius;
            glVertex3f(halfThickness, y, z);
        }
        glEnd();

        // back face (x = -halfThickness)
        glBegin(GL_TRIANGLE_FAN);
        glVertex3f(-halfThickness, 0.0f, 0.0f);
        for (int i = 0; i <= segments; ++i) {
            float a = (float)i / (float)segments * -2.0f * 3.14159265f; // reversed winding
            float y = std::cos(a) * radius;
            float z = std::sin(a) * radius;
            glVertex3f(-halfThickness, y, z);
        }
        glEnd();

        // side (connect faces)
        glBegin(GL_TRIANGLE_STRIP);
        for (int i = 0; i <= segments; ++i) {
            float a = (float)i / (float)segments * 2.0f * 3.14159265f;
            float y = std::cos(a) * radius;
            float z = std::sin(a) * radius;
            glVertex3f(-halfThickness, y, z);
            glVertex3f( halfThickness, y, z);
        }
        glEnd();
    };

    int idx = 0;
    for (const auto &c : s_coins) {
        ++idx;
        if (c.collected) continue;
        float baseY = getTerrainHeight(c.p.x, c.p.y);
        // bobbing amplitude scaled to coin size
        float coinRadius = 0.42f;
        float bob = 0.12f * std::sin((float)t * 3.0f + idx * 0.47f);
        // position coin so it stands on the terrain (bottom touches terrain)
        float y = baseY + coinRadius + bob;
        float spin = (float)t * 60.0f + idx * 11.0f; // degrees
        glPushMatrix();
        glTranslatef(c.p.x, y, c.p.y);
        // spin around vertical axis
        glRotatef(spin, 0.0f, 1.0f, 0.0f);
        // coin faces are in Y-Z plane (vertical), thickness along X
        glColor3f(0.95f, 0.8f, 0.1f);
        drawVerticalCoin(coinRadius, 0.04f, 16);  // Reduced from 48 to 16 for performance
        // small highlight: a slightly smaller lighter disc on the front face
        glPushMatrix();
        glTranslatef(0.045f, 0.0f, 0.0f); // place on front face
        glColor3f(1.0f, 0.95f, 0.6f);
        // draw a slightly smaller circular patch
        glBegin(GL_TRIANGLE_FAN);
        glVertex3f(0.0f, 0.0f, 0.0f);
        for (int i = 0; i <= 12; ++i) {  // Reduced from 48 to 12 for performance
            float a = (float)i / 12.0f * 2.0f * 3.14159265f;
            float yv = std::cos(a) * (coinRadius * 0.6f);
            float zv = std::sin(a) * (coinRadius * 0.6f);
            glVertex3f(0.0f, yv, zv);
        }
        glEnd();
        glPopMatrix();
        glPopMatrix();
        // restore coin color for next coin
        glColor3f(0.95f, 0.8f, 0.1f);
    }
}

// check for pickups
int collectCoinsAt(float x, float z, float pickupRadius) {
    int collected = 0;
    for (auto &c : s_coins) {
        if (c.collected) continue;
        float dx = x - c.p.x; float dz = z - c.p.y;
        float d2 = dx*dx + dz*dz;
        if (d2 <= pickupRadius * pickupRadius) { c.collected = true; ++collected; }
    }
    return collected;
}

static void ensureBuildingsInitialized() {
    if (!s_buildings.empty()) return;
    s_buildings.push_back(BuildingDef{-4.0f, -4.0f, 2.0f, 3.0f, 2.0f, glm::vec3(0.95f,0.95f,0.6f)});
    s_buildings.push_back(BuildingDef{6.0f, 4.0f, 1.8f, 2.5f, 1.8f, glm::vec3(0.9f,0.9f,0.5f)});
    s_buildings.push_back(BuildingDef{8.5f, 6.5f, 1.6f, 2.0f, 1.6f, glm::vec3(0.95f,0.9f,0.55f)});
}

// forward declare building draw helper
static void drawBuildingAt(float wx, float wz, float bw, float bh, float bd, const glm::vec3 &windowColor);

void drawBuildings() {
    ensureBuildingsInitialized();
    for (const auto &b : s_buildings) drawBuildingAt(b.x, b.z, b.bw, b.bh, b.bd, b.windowColor);
}

void addBuilding(const BuildingDef &b) { s_buildings.push_back(b); }
void clearBuildings() { s_buildings.clear(); }

const std::vector<BuildingDef>& getBuildings() { ensureBuildingsInitialized(); return s_buildings; }

bool isPositionInsideBuilding(float x, float z, float radius) {
    ensureBuildingsInitialized();
    for (const auto &b : s_buildings) {
        float halfW = b.bw * 0.5f;
        float halfD = b.bd * 0.5f;
        float dx = std::fabs(x - b.x);
        float dz = std::fabs(z - b.z);
        if (dx <= halfW + radius && dz <= halfD + radius) return true;
    }
    return false;
}

void addRoad(const Road &r) { s_roads.push_back(r); }
void clearRoads() { s_roads.clear(); }

const std::vector<Road>& getRoads() { return s_roads; }

const std::vector<glm::vec2>& getTrees() { ensureTreesInitialized(); return s_trees; }

const std::vector<glm::vec3>& getStreetLights() { return s_streetLights; }

const std::vector<glm::vec2>& getCoins() {
    std::vector<glm::vec2> coins;
    for (const auto &c : s_coins) {
        if (!c.collected) coins.push_back(c.p);
    }
    // Since it's static, but to avoid copying, perhaps make it return vector<vec2> of uncollected.
    // But for simplicity, return all.
    static std::vector<glm::vec2> coinPos;
    coinPos.clear();
    for (const auto &c : s_coins) {
        if (!c.collected) coinPos.push_back(c.p);
    }
    return coinPos;
}

void drawRoads() {
    const float sampleSpacing = 0.5f;
    const float sidewalkWidth = 0.45f;
    for (const auto &road : s_roads) {
        const auto &waypoints = road.pts;
        float roadHalfWidth = road.halfWidth;
        if (waypoints.size() < 2) continue;
        std::vector<glm::vec3> samples;
        for (size_t i = 0; i + 1 < waypoints.size(); ++i) {
            float x0 = waypoints[i].x, z0 = waypoints[i].y;
            float x1 = waypoints[i+1].x, z1 = waypoints[i+1].y;
            // finer sampling for main roads
            float localSpacing = road.isMain ? (sampleSpacing * 0.6f) : sampleSpacing;
            float dx = x1 - x0, dz = z1 - z0;
            float segLen = std::sqrt(dx*dx + dz*dz);
            int steps = std::max(1, (int)std::ceil(segLen / localSpacing));
            for (int s = 0; s <= steps; ++s) {
                float t = (float)s / (float)steps;
                float x = x0 + dx * t;
                float z = z0 + dz * t;
                float y = getTerrainHeight(x, z) + 0.02f;
                samples.push_back(glm::vec3(x,y,z));
            }
        }
        if (samples.size() < 2) continue;

        // remove near-duplicate samples (can occur at segment joins)
        std::vector<glm::vec3> uniq;
        const float dedupEps = 1e-3f;
        for (const auto &p : samples) {
            if (uniq.empty() || glm::length(uniq.back() - p) > dedupEps) uniq.push_back(p);
        }
        if (uniq.size() < 2) continue;

        // road surface (use deduped samples) - asphalt color
        glColor3f(0.20f,0.205f,0.22f);
        glBegin(GL_TRIANGLE_STRIP);
        for (size_t i = 0; i < uniq.size(); ++i) {
            glm::vec3 prev = (i==0) ? uniq[i] : uniq[i-1];
            glm::vec3 next = (i+1==uniq.size()) ? uniq[i] : uniq[i+1];
            glm::vec3 vec = next - prev;
            float vecLen = glm::length(vec);
            glm::vec3 tangent;
            if (vecLen < 1e-5f) {
                if (i > 0) {
                    glm::vec3 pprev = (i >= 2) ? uniq[i-2] : prev;
                    glm::vec3 v2 = prev - pprev;
                    float l2 = glm::length(v2);
                    tangent = (l2 < 1e-5f) ? glm::vec3(1.0f,0.0f,0.0f) : v2 / l2;
                } else tangent = glm::vec3(1.0f,0.0f,0.0f);
            } else tangent = vec / vecLen;
            glm::vec3 perp(-tangent.z, 0.0f, tangent.x);
            glm::vec3 left = uniq[i] + perp * roadHalfWidth;
            glm::vec3 right = uniq[i] - perp * roadHalfWidth;
            glVertex3f(left.x,left.y,left.z);
            glVertex3f(right.x,right.y,right.z);
        }
        glEnd();

        // sidewalks (concrete)
        glColor3f(0.76f,0.76f,0.74f);
        glBegin(GL_TRIANGLE_STRIP);
        for (size_t i=0;i<uniq.size();++i){
            glm::vec3 prev = (i==0) ? uniq[i] : uniq[i-1];
            glm::vec3 next = (i+1==uniq.size()) ? uniq[i] : uniq[i+1];
            glm::vec3 vec = next - prev;
            float l = glm::length(vec);
            glm::vec3 tangent = (l < 1e-5f) ? glm::vec3(1.0f,0.0f,0.0f) : vec / l;
            glm::vec3 perp(-tangent.z,0.0f,tangent.x);
            glm::vec3 outer = uniq[i] + perp * (roadHalfWidth + sidewalkWidth);
            glm::vec3 inner = uniq[i] + perp * roadHalfWidth;
            glVertex3f(outer.x, outer.y + 0.005f, outer.z);
            glVertex3f(inner.x, inner.y + 0.005f, inner.z);
        }
        glEnd();
        glBegin(GL_TRIANGLE_STRIP);
        for (size_t i=0;i<uniq.size();++i){
            glm::vec3 prev = (i==0) ? uniq[i] : uniq[i-1];
            glm::vec3 next = (i+1==uniq.size()) ? uniq[i] : uniq[i+1];
            glm::vec3 vec = next - prev;
            float l = glm::length(vec);
            glm::vec3 tangent = (l < 1e-5f) ? glm::vec3(1.0f,0.0f,0.0f) : vec / l;
            glm::vec3 perp(-tangent.z,0.0f,tangent.x);
            glm::vec3 inner = uniq[i] - perp * roadHalfWidth;
            glm::vec3 outer = uniq[i] - perp * (roadHalfWidth + sidewalkWidth);
            glVertex3f(inner.x, inner.y + 0.005f, inner.z);
            glVertex3f(outer.x, outer.y + 0.005f, outer.z);
        }
        glEnd();

    // center dashed line for non-main roads; solid for main roads
        glColor3f(1.0f,1.0f,1.0f);
        if (!road.isMain) {
            const float dashLen = 0.8f, dashGap = 0.6f;
            for (size_t i=1;i<uniq.size();++i){
                glm::vec3 a = uniq[i-1]; glm::vec3 b = uniq[i];
                glm::vec3 diff = b - a;
                float seg = glm::length(diff);
                if (seg < 1e-6f) continue;
                glm::vec3 dir = diff / seg;
                float t = 0.0f;
                while (t < seg){
                    float take = std::min(dashLen, seg - t);
                    glm::vec3 p0 = a + dir * t;
                    glm::vec3 p1 = a + dir * (t + take);
                    glm::vec3 perp(-dir.z,0.0f,dir.x);
                    float half = 0.06f;
                    glBegin(GL_QUADS);
                    glVertex3f(p0.x - perp.x*half, p0.y + 0.02f, p0.z - perp.z*half);
                    glVertex3f(p0.x + perp.x*half, p0.y + 0.02f, p0.z + perp.z*half);
                    glVertex3f(p1.x + perp.x*half, p1.y + 0.02f, p1.z + perp.z*half);
                    glVertex3f(p1.x - perp.x*half, p1.y + 0.02f, p1.z - perp.z*half);
                    glEnd();
                    t += take + dashGap;
                }
            }
        } else {
            // solid center for main roads
            glBegin(GL_TRIANGLE_STRIP);
            for (size_t i=0;i<uniq.size();++i){
                glm::vec3 prev = (i==0) ? uniq[i] : uniq[i-1];
                glm::vec3 next = (i+1==uniq.size()) ? uniq[i] : uniq[i+1];
                glm::vec3 vec = next - prev;
                float l = glm::length(vec);
                glm::vec3 dir = (l < 1e-5f) ? glm::vec3(1.0f,0.0f,0.0f) : vec / l;
                glm::vec3 perp(-dir.z,0.0f,dir.x);
                glm::vec3 cLeft = uniq[i] + perp * 0.2f;
                glm::vec3 cRight = uniq[i] - perp * 0.2f;
                glVertex3f(cLeft.x, cLeft.y + 0.02f, cLeft.z);
                glVertex3f(cRight.x, cRight.y + 0.02f, cRight.z);
            }
            glEnd();
        }

    // Draw curb/edge lines for a cleaner look
    glLineWidth(2.0f);
    // left curb (light concrete contrast)
    glColor3f(0.9f,0.9f,0.9f);
        glBegin(GL_LINE_STRIP);
        for (size_t i=0;i<uniq.size();++i){
            glm::vec3 prev = (i==0) ? uniq[i] : uniq[i-1];
            glm::vec3 next = (i+1==uniq.size()) ? uniq[i] : uniq[i+1];
            glm::vec3 vec = next - prev;
            float l = glm::length(vec);
            glm::vec3 tangent = (l < 1e-5f) ? glm::vec3(1.0f,0.0f,0.0f) : vec / l;
            glm::vec3 perp(-tangent.z,0.0f,tangent.x);
            glm::vec3 left = uniq[i] + perp * (roadHalfWidth + 0.01f);
            glVertex3f(left.x, left.y + 0.03f, left.z);
        }
        glEnd();
        // right curb
        glBegin(GL_LINE_STRIP);
        for (size_t i=0;i<uniq.size();++i){
            glm::vec3 prev = (i==0) ? uniq[i] : uniq[i-1];
            glm::vec3 next = (i+1==uniq.size()) ? uniq[i] : uniq[i+1];
            glm::vec3 vec = next - prev;
            float l = glm::length(vec);
            glm::vec3 tangent = (l < 1e-5f) ? glm::vec3(1.0f,0.0f,0.0f) : vec / l;
            glm::vec3 perp(-tangent.z,0.0f,tangent.x);
            glm::vec3 right = uniq[i] - perp * (roadHalfWidth + 0.01f);
            glVertex3f(right.x, right.y + 0.03f, right.z);
        }
        glEnd();
        glLineWidth(1.0f);

        // Plant 4 trees per road at fairly spaced locations, alternating sides
        const int treesPerRoad = 4;
        // compute total length
        float totalLen = 0.0f;
        for (size_t i=1;i<uniq.size();++i) totalLen += glm::length(uniq[i] - uniq[i-1]);
        if (totalLen > 1e-4f) {
            std::vector<float> ts;
            for (int k=1;k<=treesPerRoad;++k) ts.push_back((float)k/(treesPerRoad+1)); // e.g., 0.2,0.4,0.6,0.8
            // Before placing trees, draw an intersection cap at each original waypoint so roads connect cleanly
            for (const auto &wp : road.pts) {
                float capR = road.halfWidth + sidewalkWidth + 0.02f;
                // intersection cap matches asphalt
                glColor3f(0.20f, 0.205f, 0.22f);
                drawFilledDisk(wp.x, wp.y, capR, 24);
                // slightly lighter concrete skirt
                glColor3f(0.76f, 0.76f, 0.74f);
                drawFilledDisk(wp.x, wp.y, capR + 0.02f, 20);
            }

            for (int k=0;k<treesPerRoad;++k) {
                float target = ts[k] * totalLen;
                // find segment containing target
                float acc = 0.0f; bool placed=false;
                for (size_t i=1;i<uniq.size() && !placed;++i){
                    float seg = glm::length(uniq[i] - uniq[i-1]);
                    if (acc + seg >= target){
                        float localT = (target - acc) / seg;
                        glm::vec3 p = uniq[i-1] + (uniq[i] - uniq[i-1]) * localT;
                        glm::vec3 dir = glm::normalize(uniq[i] - uniq[i-1]);
                        glm::vec3 perp(-dir.z, 0.0f, dir.x);
                        // alternate sides
                        float side = (k % 2 == 0) ? 1.0f : -1.0f;
                        float treeOffset = roadHalfWidth + sidewalkWidth + 1.0f;
                        glm::vec3 tp = p + perp * (side * treeOffset);
                        // ensure tree is not on ANY road (check all s_roads segments)
                        glm::vec2 t2(tp.x, tp.z);
                        bool tooClose = false;
                        for (const auto &other : s_roads) {
                            for (size_t si = 1; si < other.pts.size(); ++si) {
                                float d = pointSegDist2D(t2, other.pts[si-1], other.pts[si]);
                                // strict check: tree center must be outside the road surface.
                                // consider the road half-width plus a small safety margin (no sidewalk included)
                                const float safetyMargin = 0.05f;
                                if (d <= (other.halfWidth + safetyMargin)) { tooClose = true; break; }
                            }
                            if (tooClose) break;
                        }
                        if (!tooClose) {
                            // draw a tree at tp.x,tp.z (use drawTree helper)
                            drawTree(tp.x, tp.z);
                        }
                        placed = true;
                    }
                    acc += seg;
                }
            }
        }
    }
}

// Building drawing helper (windows + frame + roof)
static void drawBuildingAt(float wx, float wz, float bw, float bh, float bd, const glm::vec3 &windowColor){
    float wy = getTerrainHeight(wx, wz);
    glPushMatrix();
    glTranslatef(wx, wy, wz);

    // body with optional texture based on building type
    // For now, randomly use type 0, 1, or 2 based on position (deterministic)
    int buildingType = ((int)(wx + wz) % 3); // 0=none, 1=brick, 2=metal
    
    if (buildingType > 0 && g_buildingTextures[buildingType]) {
        glEnable(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, g_buildingTextures[buildingType]);
        glColor3f(1.0f, 1.0f, 1.0f);
    } else {
        glColor3f(0.6f, 0.6f, 0.6f);
    }
    drawCube(bw, bh, bd, true);
    if (buildingType > 0 && g_buildingTextures[buildingType]) {
        glBindTexture(GL_TEXTURE_2D, 0);
        glDisable(GL_TEXTURE_2D);
    }

    float halfW = bw * 0.5f, halfH = bh * 0.5f, halfD = bd * 0.5f;
    int rows = std::max(1, (int)std::floor(bh));
    int cols = 2;
    float ww = std::min(0.5f, bw * 0.25f);
    float wh = std::min(0.6f, bh * 0.18f);

    // glass - batch all windows into single draw call for better performance
    glColor3f(windowColor.r, windowColor.g, windowColor.b);
    glBegin(GL_QUADS);
    for (int row = 0; row < rows; ++row) {
        for (int col = 0; col < cols; ++col) {
            float lx = -halfW + 0.6f + col * (bw - 1.2f);
            float ly = -halfH + 0.6f + row * 0.9f;
            float zoff = halfD + 0.002f;
            glVertex3f(lx - ww*0.5f, ly - wh*0.5f,  zoff);
            glVertex3f(lx + ww*0.5f, ly - wh*0.5f,  zoff);
            glVertex3f(lx + ww*0.5f, ly + wh*0.5f,  zoff);
            glVertex3f(lx - ww*0.5f, ly + wh*0.5f,  zoff);
            // back window
            glVertex3f(-lx - ww*0.5f, ly - wh*0.5f, -zoff);
            glVertex3f(-lx + ww*0.5f, ly - wh*0.5f, -zoff);
            glVertex3f(-lx + ww*0.5f, ly + wh*0.5f, -zoff);
            glVertex3f(-lx - ww*0.5f, ly + wh*0.5f, -zoff);
        }
    }
    glEnd();

    // frame lines
    glColor3f(0.1f,0.1f,0.1f);
    glBegin(GL_LINES);
    // vertical frame
    glVertex3f(-halfW, -halfH, halfD + 0.003f); glVertex3f(-halfW, halfH, halfD + 0.003f);
    glVertex3f( halfW, -halfH, halfD + 0.003f); glVertex3f( halfW, halfH, halfD + 0.003f);
    glEnd();

    // roof
    glColor3f(0.4f,0.2f,0.2f);
    glBegin(GL_TRIANGLES);
    glVertex3f(-halfW, halfH, -halfD);
    glVertex3f( halfW, halfH, -halfD);
    glVertex3f( 0.0f, halfH + 0.6f, 0.0f);
    glVertex3f(-halfW, halfH, halfD);
    glVertex3f( halfW, halfH, halfD);
    glVertex3f( 0.0f, halfH + 0.6f, 0.0f);
    glEnd();

    glPopMatrix();
}

