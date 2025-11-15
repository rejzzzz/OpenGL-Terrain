#include "../../include/city/City.h"
#include "../../include/objects.h"
#include <random>
#include <cmath>

static std::mt19937_64 rng(123456);

void clearCity() {
    clearBuildings();
}

void generateCity(int nHouses, float areaRadius, const glm::vec2 &lakeCenter) {
    clearCity();
    // first create five main roads (store locally so we can test placement)
    clearRoads();
    
    // Define lake location away from center
    glm::vec2 lakePos(-25.0f, 25.0f);  // Top-left corner area
    float lakeRad = 10.0f;
    
    std::vector<Road> mainRoads;
    Road r1; r1.halfWidth = 3.0f; r1.isMain = true; r1.pts.push_back(glm::vec2(0.0f, -areaRadius)); r1.pts.push_back(glm::vec2(0.0f, areaRadius)); mainRoads.push_back(r1);
    Road r2; r2.halfWidth = 3.0f; r2.isMain = true; r2.pts.push_back(glm::vec2(-areaRadius, 0.0f)); r2.pts.push_back(glm::vec2(areaRadius, 0.0f)); mainRoads.push_back(r2);
    Road r3; r3.halfWidth = 3.2f; r3.isMain = true; r3.pts.push_back(glm::vec2(-areaRadius, areaRadius)); r3.pts.push_back(glm::vec2(areaRadius, -areaRadius)); mainRoads.push_back(r3);
    Road r4; r4.halfWidth = 3.2f; r4.isMain = true; r4.pts.push_back(glm::vec2(-areaRadius, -areaRadius)); r4.pts.push_back(glm::vec2(areaRadius, areaRadius)); mainRoads.push_back(r4);
    Road r5; r5.halfWidth = 3.0f; r5.isMain = true; r5.pts.push_back(glm::vec2(areaRadius * 0.3f, -areaRadius)); r5.pts.push_back(glm::vec2(areaRadius * 0.3f, areaRadius)); mainRoads.push_back(r5);
    // Add some side roads for village feel - but avoid the lake area
    Road r6; r6.halfWidth = 2.0f; r6.pts.push_back(glm::vec2(-areaRadius*0.5f, -areaRadius*0.5f)); r6.pts.push_back(glm::vec2(areaRadius*0.5f, areaRadius*0.5f)); mainRoads.push_back(r6);
    Road r7; r7.halfWidth = 2.0f; r7.pts.push_back(glm::vec2(-areaRadius*0.5f, areaRadius*0.5f)); r7.pts.push_back(glm::vec2(areaRadius*0.5f, -areaRadius*0.5f)); mainRoads.push_back(r7);
    for (const auto &mr : mainRoads) addRoad(mr);

    // helper: distance from point p to segment ab
    auto pointSegDist = [](const glm::vec2 &p, const glm::vec2 &a, const glm::vec2 &b) {
        glm::vec2 v = b - a;
        glm::vec2 w = p - a;
        auto dot = [](const glm::vec2 &u, const glm::vec2 &v){ return u.x * v.x + u.y * v.y; };
        auto len = [](const glm::vec2 &u){ return std::sqrt(u.x*u.x + u.y*u.y); };
        float c1 = dot(w, v);
        if (c1 <= 0.0f) return len(p - a);
        float c2 = dot(v, v);
        if (c2 <= 0.0f) return len(p - a);
        float t = c1 / c2;
        if (t < 0.0f) t = 0.0f; else if (t > 1.0f) t = 1.0f;
        glm::vec2 proj = a + v * t;
        return len(p - proj);
    };

    // Now place buildings in positions that don't overlap any main road
    std::uniform_real_distribution<float> angDist(0.0f, 2.0f * 3.14159265f);
    std::normal_distribution<float> radDist(0.0f, areaRadius / 3.0f);
    std::uniform_real_distribution<float> sizeDist(1.0f, 3.0f);
    std::uniform_real_distribution<float> heightChoice(0.0f, 1.0f);

    int placed = 0;
    int attempts = 0;
    const int maxAttempts = nHouses * 10 + 500;
    const float safetyMargin = 0.6f;
    while (placed < nHouses && attempts < maxAttempts) {
        ++attempts;
        float a = angDist(rng);
        float r = std::abs(radDist(rng));
        if (r < areaRadius * 0.05f || r > areaRadius * 0.9f) continue;
        float x = r * std::cos(a);
        float z = r * std::sin(a);
        x += (float)(std::sin((double)attempts*7.3) * 0.4);
        z += (float)(std::cos((double)attempts*11.1) * 0.4);

        float bw = sizeDist(rng);
        float bd = sizeDist(rng) * 0.9f;
        float hroll = heightChoice(rng);
        float bh = (hroll < 0.12f) ? (6.0f + sizeDist(rng) * 2.0f) : ((hroll < 0.6f) ? (3.0f + sizeDist(rng)) : (2.0f + sizeDist(rng)*0.5f));

        glm::vec2 p(x,z);
        float halfExtent = std::max(bw, bd) * 0.5f;
        
        // Check distance from lake - ensure no building is placed near it
        float distToLake = std::sqrt((x - lakePos.x)*(x - lakePos.x) + (z - lakePos.y)*(z - lakePos.y));
        if (distToLake < lakeRad + 5.0f) continue;  // 5 units buffer around lake
        
        bool ok = true;
        for (const auto &mr : mainRoads) {
            for (size_t i = 1; i < mr.pts.size(); ++i) {
                float dist = pointSegDist(p, mr.pts[i-1], mr.pts[i]);
                if (dist <= (mr.halfWidth + halfExtent + safetyMargin)) { ok = false; break; }
            }
            if (!ok) break;
        }
        if (!ok) continue;

        glm::vec3 wc(0.95f, 0.9f, 0.55f);
        BuildingDef b{ x, z, bw, bh, bd, wc };
        addBuilding(b);
        ++placed;
    }

    // Add a lake at the requested center (if caller provided a meaningful center)
    float lakeRadius = std::max(4.0f, areaRadius * 0.25f);
    // if lakeCenter is near origin (0,0) when caller didn't set, keep previous placement
    auto vlen = [](const glm::vec2 &v){ return std::sqrt(v.x*v.x + v.y*v.y); };
    // Add an isolated lake away from the village
    addPond(lakePos, lakeRad);

    // Add 4 street lights along the offset vertical road (mainRoads[4]) at far distances
    // But avoid placing them near the lake
    if (mainRoads.size() >= 5) {
        float x = mainRoads[4].pts[0].x; // vertical line x coordinate
        std::vector<float> zPositions = {-areaRadius * 0.9f, -areaRadius * 0.3f, areaRadius * 0.3f, areaRadius * 0.9f};
        for (float z : zPositions) {
            float distToLake = std::sqrt((x - lakePos.x)*(x - lakePos.x) + (z - lakePos.y)*(z - lakePos.y));
            if (distToLake > lakeRad + 5.0f) {  // Only add if far from lake
                addStreetLight(glm::vec3(x, 0.0f, z));
            }
        }
    }
}
