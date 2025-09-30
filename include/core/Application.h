#pragma once
#include <GL/glew.h> // Must be included before GLFW (it includes gl.h)
#include <GLFW/glfw3.h>
#include <memory>

class Scene;

// Handles window/context lifecycle and main loop dispatch.
class Application {
public:
    Application(int width, int height, const char* title);
    ~Application();

    bool Init();
    void Run();

    void SetScene(std::unique_ptr<Scene> scene);

    GLFWwindow* GetWindow() const { return m_Window; }

private:
    int m_Width, m_Height;
    const char* m_Title;
    GLFWwindow* m_Window{};
    std::unique_ptr<Scene> m_Scene;
};
