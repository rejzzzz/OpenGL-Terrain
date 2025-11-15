#include "../../include/scenes/PlayScene.h"
#include <GL/glu.h>
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>
#include <cmath>

#include <vector>
#include <string>
#include "skybox/skybox.h"
#include "../../include/city/City.h"
#include "../../include/objects.h"

PlayScene::PlayScene()
    : m_Player(0.0f,0.0f,0.0f), m_Camera(&m_Player) {
        std::vector<std::string> faces;
    
    // This is the path to the folder you created in the last step
    std::string skyboxDir = "assets/skybox/";

    // The order MUST be: Right, Left, Top, Bottom, Front, Back
    faces.push_back(skyboxDir + "right.png");
    faces.push_back(skyboxDir + "left.png");
    faces.push_back(skyboxDir + "top.png");
    faces.push_back(skyboxDir + "bottom.png");
    faces.push_back(skyboxDir + "front.png");
    faces.push_back(skyboxDir + "back.png");
    
    loadSkybox(faces);

    generateCity(50, 40.0f, glm::vec2(0,0));
}

void PlayScene::OnAttach(GLFWwindow* window) {
    m_Window = window;
    int w,h; glfwGetFramebufferSize(window,&w,&h);
    OnFramebufferResize(w,h);
    std::cout << "Controls:\n  WASD move\n  RMB drag orbit\n  Scroll zoom\n  ESC quit\n";

    glEnable(GL_COLOR_MATERIAL);
    glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);

    // Add multiple mountains across the terrain for variety
    // Keep the mountain list empty here; border mountains are added below to frame the scene
    terrainClearMountains();

    // --- Border mountains recalculated to match the current terrain extents ---
    const int TERRAIN_SIZE = 200; // must match terrain.cpp
    const float SPACING = 0.5f;
    const float halfWorld = (TERRAIN_SIZE / 2) * SPACING; // e.g. 100 * 0.5 = 50.0
    const float minEdge = -halfWorld;
    const float maxEdge = (TERRAIN_SIZE/2 - 1) * SPACING;
    // place mountains just inside the perimeter with some spread
    // left column (x ~ minEdge + 3)
    terrainAddMountain(glm::vec2(minEdge + 3.0f, -halfWorld * 0.6f), 12.0f, 4.2f);
    terrainAddMountain(glm::vec2(minEdge + 3.0f, 0.0f), 14.0f, 5.0f);
    terrainAddMountain(glm::vec2(minEdge + 3.0f, halfWorld * 0.6f), 12.0f, 4.0f);
    // top row (z ~ minEdge + 3)
    terrainAddMountain(glm::vec2(-halfWorld * 0.6f, minEdge + 3.0f), 10.0f, 3.6f);
    terrainAddMountain(glm::vec2(0.0f, minEdge + 3.0f), 16.0f, 5.2f);
    terrainAddMountain(glm::vec2(halfWorld * 0.6f, minEdge + 3.0f), 10.0f, 3.6f);
    // right column (x ~ maxEdge - 3)
    terrainAddMountain(glm::vec2(maxEdge - 3.0f, -halfWorld * 0.6f), 12.0f, 4.0f);
    terrainAddMountain(glm::vec2(maxEdge - 3.0f, 0.0f), 14.0f, 4.8f);
    terrainAddMountain(glm::vec2(maxEdge - 3.0f, halfWorld * 0.6f), 12.0f, 4.0f);
    // bottom row (z ~ maxEdge - 3)
    terrainAddMountain(glm::vec2(halfWorld * 0.6f, maxEdge - 3.0f), 10.0f, 3.4f);
    terrainAddMountain(glm::vec2(0.0f, maxEdge - 3.0f), 16.0f, 5.0f);
    terrainAddMountain(glm::vec2(-halfWorld * 0.6f, maxEdge - 3.0f), 10.0f, 3.4f);

    // Generate a simple city with ~30 houses around the origin
    // Place the lake between two chosen border mountains for a scenic look
    glm::vec2 mountainA(minEdge + 3.0f, 0.0f); // left-column middle mountain
    glm::vec2 mountainB(-halfWorld * 0.6f, minEdge + 3.0f); // top-row left mountain
    glm::vec2 lakeCenter = (mountainA + mountainB) * 0.5f;
    generateCity(30, 40.0f, lakeCenter);
    // spawn collectible coins around the city
    spawnCoins(60, 40.0f);

}

void PlayScene::OnFramebufferResize(int width, int height) {
    if (height<=0) height = 1;
    glViewport(0,0,width,height);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    float aspect = (float)width/(float)height;
    glFrustum(-0.1f*aspect,0.1f*aspect,-0.1f,0.1f,0.2f,100.0f);
    glMatrixMode(GL_MODELVIEW);
}

void PlayScene::OnKey(int key, int scancode, int action, int mods) {
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(m_Window,true);
    // Update movement input flags on press/release
    if (key == GLFW_KEY_W) {
        m_MoveForward = (action != GLFW_RELEASE);
    } else if (key == GLFW_KEY_S) {
        m_MoveBack = (action != GLFW_RELEASE);
    } else if (key == GLFW_KEY_A) {
        m_MoveLeft = (action != GLFW_RELEASE);
    } else if (key == GLFW_KEY_D) {
        m_MoveRight = (action != GLFW_RELEASE);
    }
}

void PlayScene::OnMouseButton(int button, int action, int mods) {
    if (button == GLFW_MOUSE_BUTTON_RIGHT) {
        if (action == GLFW_PRESS) { m_RMouseDown = true; m_FirstMouse = true; }
        else if (action == GLFW_RELEASE) { m_RMouseDown = false; }
    }
}

void PlayScene::OnCursorPos(double x, double y) {
    if (!m_RMouseDown) { m_LastX = x; m_LastY = y; return; }
    if (m_FirstMouse) { m_LastX = x; m_LastY = y; m_FirstMouse = false; return; }
    float dx = (float)(x - m_LastX);
    float dy = (float)(y - m_LastY);
    m_LastX = x; m_LastY = y;
    m_Camera.ProcessMouseMovement(dx, dy);
}

void PlayScene::OnScroll(double xoff, double yoff) {
    m_Camera.ProcessScroll((float)yoff);
}

void PlayScene::OnUpdate(float dt) {
    m_Camera.Update();
    // Build desired movement from input flags for smooth walking
    glm::vec3 forward = m_Camera.GetForward(); forward.y = 0; if (glm::length(forward) > 0.0001f) forward = glm::normalize(forward);
    glm::vec3 right = m_Camera.GetRight(); right.y = 0; if (glm::length(right) > 0.0001f) right = glm::normalize(right);
    glm::vec3 desiredDir(0.0f);
    if (m_MoveForward) desiredDir += forward;
    if (m_MoveBack) desiredDir -= forward;
    if (m_MoveLeft) desiredDir -= right;
    if (m_MoveRight) desiredDir += right;
    float speed = 3.2f; // world units per second
    if (glm::length(desiredDir) > 0.0001f) {
        desiredDir = glm::normalize(desiredDir);
        m_Player.SetDesiredMovement(desiredDir, speed);
    } else {
        m_Player.SetDesiredMovement(glm::vec3(0.0f), 0.0f);
    }

    // Face player toward camera forward when the user is dragging the camera (RMB),
    // otherwise face movement direction if moving.
    if (m_RMouseDown) {
        // instant facing: set player yaw immediately to camera forward
        glm::vec3 cf = m_Camera.GetForward(); cf.y = 0.0f;
        if (glm::dot(cf, cf) > 0.0001f) {
            cf = glm::normalize(cf);
            float yawDeg = glm::degrees(std::atan2(cf.x, -cf.z));
            m_Player.SetYaw(yawDeg);
        }
    } else if (glm::length(desiredDir) > 0.0001f) {
        glm::vec3 md = desiredDir; md.y = 0.0f; md = glm::normalize(md);
        float yawDeg = glm::degrees(std::atan2(md.x, -md.z));
        m_Player.SetTargetYaw(yawDeg);
    }

    // Update player physics / animation with dt
    m_Player.Update(dt);

    // Check coin pickups at player position
    glm::vec3 pp = m_Player.GetPosition();
    int got = collectCoinsAt(pp.x, pp.z, 0.9f);
    (void)got;
}

void PlayScene::OnRender() {
    // --- THIS IS THE MOST IMPORTANT FIX ---
    // Clear both the color and the depth buffer from the last frame.
    // This stops the "blue bar" and allows the terrain to draw correctly.
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    // --- END OF FIX ---

    glLoadIdentity(); // Resets the matrix

    // Calculate the camera's target point ONCE
    glm::vec3 center = m_Player.GetPosition(); center.y += 1.0f;
    
    // Draw the skybox first
    drawSkybox(m_Camera, center);

    // This is your original code to set up the main camera for the scene
    glm::vec3 eye = m_Camera.GetPosition();
    glm::vec3 up(0,1,0);
    gluLookAt(eye.x, eye.y, eye.z, center.x, center.y, center.z, up.x, up.y, up.z);

    drawTerrain();
    // draw water bodies first (recessed), then roads, buildings, trees and street lights
    drawPonds();
    drawRoads();
    drawBuildings();
    drawTrees();
    drawStreetLights();
    drawCoins();
    m_Player.Draw();

    // Draw HUD: numeric coin counter (top-left) using a simple 7-segment style
    glDisable(GL_DEPTH_TEST);
    int w,h; glfwGetFramebufferSize(m_Window, &w, &h);
    int collected = getCollectedCoinsCount();
    int total = getTotalCoinsCount();
    // prepare orthographic 2D
    glMatrixMode(GL_PROJECTION);
    glPushMatrix(); glLoadIdentity(); glOrtho(0, w, h, 0, -1, 1);
    glMatrixMode(GL_MODELVIEW); glPushMatrix(); glLoadIdentity();

    // draw a small coin icon at left
    auto drawIcon = [&](int x, int y, int size){
        glColor3f(0.95f, 0.8f, 0.1f);
        glBegin(GL_QUADS);
        glVertex2i(x, y);
        glVertex2i(x+size, y);
        glVertex2i(x+size, y+size);
        glVertex2i(x, y+size);
        glEnd();
    };

    // 7-seg digit drawer (simple segments as rectangles)
    auto drawDigit = [&](int px, int py, int wseg, int hseg, int digit){
        // segment layout (a,b,c,d,e,f,g)
        bool seg[7]={0};
        switch(digit){
            case 0: seg[0]=seg[1]=seg[2]=seg[3]=seg[4]=seg[5]=true; break;
            case 1: seg[1]=seg[2]=true; break;
            case 2: seg[0]=seg[1]=seg[6]=seg[4]=seg[3]=true; break;
            case 3: seg[0]=seg[1]=seg[6]=seg[2]=seg[3]=true; break;
            case 4: seg[5]=seg[6]=seg[1]=seg[2]=true; break;
            case 5: seg[0]=seg[5]=seg[6]=seg[2]=seg[3]=true; break;
            case 6: seg[0]=seg[5]=seg[4]=seg[3]=seg[2]=seg[6]=true; break;
            case 7: seg[0]=seg[1]=seg[2]=true; break;
            case 8: for(int i=0;i<7;++i) seg[i]=true; break;
            case 9: seg[0]=seg[1]=seg[2]=seg[3]=seg[5]=seg[6]=true; break;
            default: break;
        }
        int sw = wseg, sh = hseg;
        // a (top)
        if(seg[0]){ glBegin(GL_QUADS); glVertex2i(px+sw, py); glVertex2i(px+sw+sh, py); glVertex2i(px+sw+sh, py+sw); glVertex2i(px+sw, py+sw); glEnd(); }
        // b (top-right)
        if(seg[1]){ glBegin(GL_QUADS); glVertex2i(px+sw+sh, py); glVertex2i(px+sw+sh+sw, py+sw); glVertex2i(px+sw+sh+sw, py+sw+sw); glVertex2i(px+sw+sh, py+sw); glEnd(); }
        // c (bottom-right)
        if(seg[2]){ glBegin(GL_QUADS); glVertex2i(px+sw+sh, py+sw+sw); glVertex2i(px+sw+sh+sw, py+sw+sw); glVertex2i(px+sw+sh+sw, py+sw+sw+sw); glVertex2i(px+sw+sh, py+sw+sw+sw); glEnd(); }
        // d (bottom)
        if(seg[3]){ glBegin(GL_QUADS); glVertex2i(px+sw, py+sw+sw+sw); glVertex2i(px+sw+sh, py+sw+sw+sw); glVertex2i(px+sw+sh, py+sw+sw+sw+sw); glVertex2i(px+sw, py+sw+sw+sw+sw); glEnd(); }
        // e (bottom-left)
        if(seg[4]){ glBegin(GL_QUADS); glVertex2i(px, py+sw+sw); glVertex2i(px+sw, py+sw+sw); glVertex2i(px+sw, py+sw+sw+sw); glVertex2i(px, py+sw+sw+sw); glEnd(); }
        // f (top-left)
        if(seg[5]){ glBegin(GL_QUADS); glVertex2i(px, py); glVertex2i(px+sw, py); glVertex2i(px+sw, py+sw); glVertex2i(px, py+sw); glEnd(); }
        // g (middle)
        if(seg[6]){ glBegin(GL_QUADS); glVertex2i(px+sw, py+sw); glVertex2i(px+sw+sh, py+sw); glVertex2i(px+sw+sh, py+sw+sw); glVertex2i(px+sw, py+sw+sw); glEnd(); }
    };

    // draw icon and numeric counter
    int iconX = 12, iconY = 12, iconSize = 20;
    drawIcon(iconX, iconY, iconSize);
    // numeric: collected / total
    std::string left = std::to_string(collected);
    std::string right = std::to_string(total);
    int dx = iconX + iconSize + 8;
    int segW = 3, segH = 10;
    glColor3f(1.0f,1.0f,1.0f);
    // draw each digit of collected
    for (size_t i=0;i<left.size();++i){ int d = left[i]-'0'; drawDigit(dx + i*(segH+segW+4), iconY, segW, segH, d); }
    // draw slash as small quad
    int slashX = dx + (int)left.size()*(segH+segW+4) + 6;
    glBegin(GL_LINES); glVertex2i(slashX, iconY+6); glVertex2i(slashX+10, iconY+iconSize-6); glEnd();
    // draw total
    int baseX = slashX + 16;
    for (size_t i=0;i<right.size();++i){ int d = right[i]-'0'; drawDigit(baseX + i*(segH+segW+4), iconY, segW, segH, d); }

    // Draw mini-map in top-right corner
    {
        int mapSize = 150;
        int mapX = w - mapSize - 10;
        int mapY = 10;
        float mapRadius = 40.0f; // world units radius
        float scale = mapSize / (2 * mapRadius);

        // Draw map background
        glColor3f(0.5f, 0.5f, 0.5f);
        glBegin(GL_QUADS);
        glVertex2i(mapX, mapY);
        glVertex2i(mapX + mapSize, mapY);
        glVertex2i(mapX + mapSize, mapY + mapSize);
        glVertex2i(mapX, mapY + mapSize);
        glEnd();

        // Get player position
        glm::vec3 playerPos = m_Player.GetPosition();
        float playerYaw = m_Player.GetYaw();

        // Draw buildings
        glColor3f(0.7f, 0.7f, 0.7f);
        const auto& buildings = getBuildings();
        for (const auto& b : buildings) {
            float dx = b.x - playerPos.x;
            float dz = b.z - playerPos.z;
            if (std::fabs(dx) > mapRadius || std::fabs(dz) > mapRadius) continue;
            float px = mapX + mapSize/2 + dx * scale;
            float pz = mapY + mapSize/2 + dz * scale;
            float bw = b.bw * scale;
            float bd = b.bd * scale;
            glBegin(GL_QUADS);
            glVertex2f(px - bw/2, pz - bd/2);
            glVertex2f(px + bw/2, pz - bd/2);
            glVertex2f(px + bw/2, pz + bd/2);
            glVertex2f(px - bw/2, pz + bd/2);
            glEnd();
        }

        // Draw roads
        glColor3f(0.3f, 0.3f, 0.3f);
        const auto& roads = getRoads();
        for (const auto& road : roads) {
            const auto& pts = road.pts;
            glBegin(GL_LINES);
            for (size_t i = 0; i + 1 < pts.size(); ++i) {
                float dx1 = pts[i].x - playerPos.x;
                float dz1 = pts[i].y - playerPos.z;
                float dx2 = pts[i+1].x - playerPos.x;
                float dz2 = pts[i+1].y - playerPos.z;
                if (std::fabs(dx1) > mapRadius || std::fabs(dz1) > mapRadius ||
                    std::fabs(dx2) > mapRadius || std::fabs(dz2) > mapRadius) continue;
                float px1 = mapX + mapSize/2 + dx1 * scale;
                float pz1 = mapY + mapSize/2 + dz1 * scale;
                float px2 = mapX + mapSize/2 + dx2 * scale;
                float pz2 = mapY + mapSize/2 + dz2 * scale;
                glVertex2f(px1, pz1);
                glVertex2f(px2, pz2);
            }
            glEnd();
        }

        // Draw trees
        glColor3f(0.0f, 0.8f, 0.0f);
        const auto& trees = getTrees();
        for (const auto& t : trees) {
            float dx = t.x - playerPos.x;
            float dz = t.y - playerPos.z;
            if (std::fabs(dx) > mapRadius || std::fabs(dz) > mapRadius) continue;
            float px = mapX + mapSize/2 + dx * scale;
            float pz = mapY + mapSize/2 + dz * scale;
            float size = 2.0f;
            glBegin(GL_QUADS);
            glVertex2f(px - size/2, pz - size/2);
            glVertex2f(px + size/2, pz - size/2);
            glVertex2f(px + size/2, pz + size/2);
            glVertex2f(px - size/2, pz + size/2);
            glEnd();
        }

        // Draw ponds
        glColor3f(0.0f, 0.0f, 1.0f);
        const auto& ponds = getPonds();
        for (const auto& pond : ponds) {
            float dx = pond.first.x - playerPos.x;
            float dz = pond.first.y - playerPos.z;
            if (std::fabs(dx) > mapRadius || std::fabs(dz) > mapRadius) continue;
            float px = mapX + mapSize/2 + dx * scale;
            float pz = mapY + mapSize/2 + dz * scale;
            float radius = pond.second * scale;
            glBegin(GL_QUADS);
            glVertex2f(px - radius, pz - radius);
            glVertex2f(px + radius, pz - radius);
            glVertex2f(px + radius, pz + radius);
            glVertex2f(px - radius, pz + radius);
            glEnd();
        }

        // Draw coins
        glColor3f(1.0f, 1.0f, 0.0f);
        const auto& coins = getCoins();
        for (const auto& c : coins) {
            float dx = c.x - playerPos.x;
            float dz = c.y - playerPos.z;
            if (std::fabs(dx) > mapRadius || std::fabs(dz) > mapRadius) continue;
            float px = mapX + mapSize/2 + dx * scale;
            float pz = mapY + mapSize/2 + dz * scale;
            float size = 2.0f;
            glBegin(GL_QUADS);
            glVertex2f(px - size/2, pz - size/2);
            glVertex2f(px + size/2, pz - size/2);
            glVertex2f(px + size/2, pz + size/2);
            glVertex2f(px - size/2, pz + size/2);
            glEnd();
        }

        // Draw street lights
        glColor3f(0.4f, 0.4f, 0.4f);
        const auto& lights = getStreetLights();
        for (const auto& l : lights) {
            float dx = l.x - playerPos.x;
            float dz = l.z - playerPos.z;
            if (std::fabs(dx) > mapRadius || std::fabs(dz) > mapRadius) continue;
            float px = mapX + mapSize/2 + dx * scale;
            float pz = mapY + mapSize/2 + dz * scale;
            float size = 1.0f;
            glBegin(GL_QUADS);
            glVertex2f(px - size/2, pz - size/2);
            glVertex2f(px + size/2, pz - size/2);
            glVertex2f(px + size/2, pz + size/2);
            glVertex2f(px - size/2, pz + size/2);
            glEnd();
        }

        // Draw player as a triangle pointing in facing direction
        glColor3f(1.0f, 0.0f, 0.0f);
        float playerSize = 10.0f;
        float px = mapX + mapSize/2;
        float pz = mapY + mapSize/2;
        float dirX = std::sin(glm::radians(playerYaw));
        float dirZ = std::cos(glm::radians(playerYaw));
        glBegin(GL_TRIANGLES);
        glVertex2f(px + dirX * playerSize, pz + dirZ * playerSize);
        glVertex2f(px + dirZ * playerSize * 0.5f - dirX * playerSize * 0.5f, pz - dirX * playerSize * 0.5f - dirZ * playerSize * 0.5f);
        glVertex2f(px - dirZ * playerSize * 0.5f - dirX * playerSize * 0.5f, pz + dirX * playerSize * 0.5f - dirZ * playerSize * 0.5f);
        glEnd();
    }

    // restore matrices
    glPopMatrix(); glMatrixMode(GL_PROJECTION); glPopMatrix(); glMatrixMode(GL_MODELVIEW);
    glEnable(GL_DEPTH_TEST);
}
// (Removed stray example code; buildings are drawn via drawBuildings())
