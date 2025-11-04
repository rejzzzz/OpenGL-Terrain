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
    std::vector<Road> mainRoads;
    Road r1; r1.halfWidth = 3.0f; r1.isMain = true; r1.pts.push_back(glm::vec2(0.0f, -areaRadius)); r1.pts.push_back(glm::vec2(0.0f, areaRadius)); mainRoads.push_back(r1);
    Road r2; r2.halfWidth = 3.0f; r2.isMain = true; r2.pts.push_back(glm::vec2(-areaRadius, 0.0f)); r2.pts.push_back(glm::vec2(areaRadius, 0.0f)); mainRoads.push_back(r2);
    Road r3; r3.halfWidth = 3.2f; r3.isMain = true; r3.pts.push_back(glm::vec2(-areaRadius, areaRadius)); r3.pts.push_back(glm::vec2(areaRadius, -areaRadius)); mainRoads.push_back(r3);
    Road r4; r4.halfWidth = 3.2f; r4.isMain = true; r4.pts.push_back(glm::vec2(-areaRadius, -areaRadius)); r4.pts.push_back(glm::vec2(areaRadius, areaRadius)); mainRoads.push_back(r4);
    Road r5; r5.halfWidth = 3.0f; r5.isMain = true; r5.pts.push_back(glm::vec2(areaRadius * 0.3f, -areaRadius)); r5.pts.push_back(glm::vec2(areaRadius * 0.3f, areaRadius)); mainRoads.push_back(r5);
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
    std::uniform_real_distribution<float> radDist(areaRadius * 0.05f, areaRadius * 0.9f);
    std::uniform_real_distribution<float> sizeDist(1.0f, 3.0f);
    std::uniform_real_distribution<float> heightChoice(0.0f, 1.0f);

    int placed = 0;
    int attempts = 0;
    const int maxAttempts = nHouses * 10 + 500;
    const float safetyMargin = 0.6f;
    while (placed < nHouses && attempts < maxAttempts) {
        ++attempts;
        float a = angDist(rng);
        float r = radDist(rng);
        float x = cosf(a) * r;
        float z = sinf(a) * r;
        x += (float)(std::sin((double)attempts*7.3) * 0.4);
        z += (float)(std::cos((double)attempts*11.1) * 0.4);

        float bw = sizeDist(rng);
        float bd = sizeDist(rng) * 0.9f;
        float hroll = heightChoice(rng);
        float bh = (hroll < 0.12f) ? (6.0f + sizeDist(rng) * 2.0f) : ((hroll < 0.6f) ? (3.0f + sizeDist(rng)) : (2.0f + sizeDist(rng)*0.5f));

        glm::vec2 p(x,z);
        float halfExtent = std::max(bw, bd) * 0.5f;
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
    // lake removed by user request - no pond will be created here
    (void)lakeRadius;
    (void)lakeCenter;

    // Add 4 street lights along the offset vertical road (mainRoads[4]) at far distances
    if (mainRoads.size() >= 5) {
        float x = mainRoads[4].pts[0].x; // vertical line x coordinate
        float z0 = -areaRadius * 0.9f;
        float z1 = -areaRadius * 0.3f;
        float z2 =  areaRadius * 0.3f;
        float z3 =  areaRadius * 0.9f;
        addStreetLight(glm::vec3(x, 0.0f, z0));
        addStreetLight(glm::vec3(x, 0.0f, z1));
        addStreetLight(glm::vec3(x, 0.0f, z2));
        addStreetLight(glm::vec3(x, 0.0f, z3));
    }
}
