#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <cmath>
#include "../include/terrain.h"
#include "../include/objects.h"

const int WIDTH = 800;
const int HEIGHT = 600;

float camAngleX = 0.0f;
float camAngleY = 0.0f;
float camDistance = 15.0f;
float lastX = WIDTH / 2.0f;
float lastY = HEIGHT / 2.0f;
bool firstMouse = true;
bool mousePressed = false;

void mouseCallback(GLFWwindow* window, double xpos, double ypos) {
    if (!mousePressed) return;
    if (firstMouse) {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }
    float xOffset = xpos - lastX;
    float yOffset = lastY - ypos;
    lastX = xpos;
    lastY = ypos;

    float sensitivity = 0.2f;
    camAngleY += xOffset * sensitivity;
    camAngleX += yOffset * sensitivity;
    if (camAngleX > 89.0f) camAngleX = 89.0f;
    if (camAngleX < -89.0f) camAngleX = -89.0f;
}

void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods) {
    if (button == GLFW_MOUSE_BUTTON_LEFT) {
        mousePressed = (action == GLFW_PRESS);
        if (mousePressed) firstMouse = true;
    }
}

void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
    if (key == GLFW_KEY_W && action == GLFW_PRESS) camDistance -= 1.0f;
    if (key == GLFW_KEY_S && action == GLFW_PRESS) camDistance += 1.0f;
}

int main() {
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW\n";
        return -1;
    }

    GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "Terrain Scene", NULL, NULL);
    if (!window) {
        std::cerr << "Failed to create window\n";
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);

    if (glewInit() != GLEW_OK) {
        std::cerr << "Failed to initialize GLEW\n";
        return -1;
    }

    glfwSetCursorPosCallback(window, mouseCallback);
    glfwSetMouseButtonCallback(window, mouseButtonCallback);
    glfwSetKeyCallback(window, keyCallback);

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_COLOR_MATERIAL); // So glColor affects lighting

    // Set projection
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    float aspect = (float)WIDTH / HEIGHT;
    glFrustum(-0.1f * aspect, 0.1f * aspect, -0.1f, 0.1f, 0.2f, 100.0f);
    glMatrixMode(GL_MODELVIEW);

    std::cout << "Controls:\n";
    std::cout << "- Left drag: rotate view\n";
    std::cout << "- W/S: zoom in/out\n";
    std::cout << "- ESC: quit\n";

    while (!glfwWindowShouldClose(window)) {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glClearColor(0.5f, 0.7f, 1.0f, 1.0f); // Sky blue

        glLoadIdentity();
        // Camera: orbit around origin
        glTranslatef(0.0f, 0.0f, -camDistance);
        glRotatef(camAngleX, 1.0f, 0.0f, 0.0f);
        glRotatef(camAngleY, 0.0f, 1.0f, 0.0f);

        drawTerrain();
        drawBuildings();
        drawTrees();

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}