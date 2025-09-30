#pragma once
#include <GLFW/glfw3.h>

// Abstract scene interface for update + render + input hooks.
class Scene {
public:
    virtual ~Scene() = default;
    virtual void OnAttach(GLFWwindow* window) {}
    virtual void OnDetach() {}
    virtual void OnUpdate(float dt) = 0;
    virtual void OnRender() = 0;
    virtual void OnFramebufferResize(int width, int height) {}
    virtual void OnKey(int key, int scancode, int action, int mods) {}
    virtual void OnMouseButton(int button, int action, int mods) {}
    virtual void OnCursorPos(double x, double y) {}
    virtual void OnScroll(double xoff, double yoff) {}
};
