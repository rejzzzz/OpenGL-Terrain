#include "../../include/scenes/PlayScene.h"
#include <GL/glu.h>
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>

#include <vector>
#include <string>
#include "skybox/skybox.h"

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
}

void PlayScene::OnAttach(GLFWwindow* window) {
    m_Window = window;
    int w,h; glfwGetFramebufferSize(window,&w,&h);
    OnFramebufferResize(w,h);
    std::cout << "Controls:\n  WASD move\n  RMB drag orbit\n  Scroll zoom\n  ESC quit\n";

    glEnable(GL_COLOR_MATERIAL);
    glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);

    // Add multiple mountains across the terrain for variety
    terrainClearMountains();
    // central ridge
    terrainAddMountain(glm::vec2(5.0f, 5.0f), 6.0f, 3.5f);
    terrainAddMountain(glm::vec2(-8.0f, 3.0f), 4.5f, 2.2f);
    // distant peaks
    terrainAddMountain(glm::vec2(12.0f, -4.0f), 7.0f, 4.0f);
    terrainAddMountain(glm::vec2(-14.0f, 10.0f), 6.5f, 3.2f);
    // smaller hill cluster
    terrainAddMountain(glm::vec2(0.0f, -10.0f), 3.5f, 1.8f);

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

    const float moveSpeed = 0.15f;
    if (action == GLFW_PRESS || action == GLFW_REPEAT) {
        glm::vec3 f = m_Camera.GetForward(); f.y = 0; float fl2=glm::dot(f,f); if(fl2>0.0001f) f=glm::normalize(f);
        glm::vec3 r = m_Camera.GetRight();   r.y = 0; float rl2=glm::dot(r,r); if(rl2>0.0001f) r=glm::normalize(r);
        switch (key) {
            case GLFW_KEY_W: m_Player.MoveInDirection(f,  moveSpeed); break;
            case GLFW_KEY_S: m_Player.MoveInDirection(-f, moveSpeed); break;
            case GLFW_KEY_A: m_Player.MoveInDirection(-r, moveSpeed); break;
            case GLFW_KEY_D: m_Player.MoveInDirection(r,  moveSpeed); break;
            default: break;
        }
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
    // Face player toward camera forward projection
    glm::vec3 f = m_Camera.GetForward(); f.y=0; float fl2=glm::dot(f,f); if(fl2>0.0001f){ f=glm::normalize(f); float yawDeg = glm::degrees(std::atan2(f.x, -f.z)); m_Player.SetYaw(yawDeg);}    
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
    // draw road on top of terrain and beneath buildings
    drawRoads();
    drawBuildings();
    drawTrees();
    m_Player.Draw();
}
// (Removed stray example code; buildings are drawn via drawBuildings())
