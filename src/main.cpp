#include "../include/core/Application.h"
#include "../include/scenes/PlayScene.h"
#include <memory>

// Global forwarding to current scene for GLFW callbacks.
static Scene* gActiveScene = nullptr;

static void framebufferSizeCallback(GLFWwindow* w, int width, int height) {
    if (gActiveScene) gActiveScene->OnFramebufferResize(width,height);
}
static void keyCallback(GLFWwindow* w, int key, int sc, int action, int mods) {
    if (gActiveScene) gActiveScene->OnKey(key,sc,action,mods);
}
static void mouseButtonCallback(GLFWwindow* w, int button, int action, int mods) {
    if (gActiveScene) gActiveScene->OnMouseButton(button,action,mods);
}
static void cursorPosCallback(GLFWwindow* w, double x, double y) {
    if (gActiveScene) gActiveScene->OnCursorPos(x,y);
}
static void scrollCallback(GLFWwindow* w, double xo, double yo) {
    if (gActiveScene) gActiveScene->OnScroll(xo,yo);
}

int main() {
    Application app(800,600,"Terrain Scene");
    if (!app.Init()) return -1;

    auto scene = std::make_unique<PlayScene>();
    gActiveScene = scene.get();

    GLFWwindow* window = app.GetWindow();
    glfwSetFramebufferSizeCallback(window, framebufferSizeCallback);
    glfwSetKeyCallback(window, keyCallback);
    glfwSetMouseButtonCallback(window, mouseButtonCallback);
    glfwSetCursorPosCallback(window, cursorPosCallback);
    glfwSetScrollCallback(window, scrollCallback);

    app.SetScene(std::move(scene));
    app.Run();
    return 0;
}