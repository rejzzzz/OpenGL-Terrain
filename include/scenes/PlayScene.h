#pragma once
#include "../core/Scene.h"
#include "../objects/MovableObject.h"
#include "../camera/Camera.h"
#include "../terrain.h"
#include "../objects.h"
#include <glm/glm.hpp>

class PlayScene : public Scene {
public:
    PlayScene();
    void OnAttach(GLFWwindow* window) override;
    void OnUpdate(float dt) override;
    void OnRender() override;
    void OnFramebufferResize(int width, int height) override;
    void OnKey(int key, int scancode, int action, int mods) override;
    void OnMouseButton(int button, int action, int mods) override;
    void OnCursorPos(double x, double y) override;
    void OnScroll(double xoff, double yoff) override;

private:
    GLFWwindow* m_Window{};
    MovableObject m_Player;
    Camera m_Camera;
    bool m_RMouseDown = false;
    bool m_FirstMouse = true;
    double m_LastX = 0.0;
    double m_LastY = 0.0;
};
