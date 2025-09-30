#include "../../include/core/Application.h"
#include "../../include/core/Scene.h"
#include <iostream>
#include <chrono>

Application::Application(int width, int height, const char* title)
    : m_Width(width), m_Height(height), m_Title(title) {}

Application::~Application() {
    if (m_Scene) m_Scene->OnDetach();
    if (m_Window) glfwDestroyWindow(m_Window);
    glfwTerminate();
}

bool Application::Init() {
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW\n";
        return false;
    }
    m_Window = glfwCreateWindow(m_Width, m_Height, m_Title, nullptr, nullptr);
    if (!m_Window) {
        std::cerr << "Failed to create window\n";
        glfwTerminate();
        return false;
    }
    glfwMakeContextCurrent(m_Window);

    if (glewInit() != GLEW_OK) {
        std::cerr << "Failed to initialize GLEW\n";
        return false;
    }
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_COLOR_MATERIAL);
    return true;
}

void Application::SetScene(std::unique_ptr<Scene> scene) {
    if (m_Scene) m_Scene->OnDetach();
    m_Scene = std::move(scene);
    if (m_Scene) m_Scene->OnAttach(m_Window);
}

void Application::Run() {
    using clock = std::chrono::high_resolution_clock;
    auto last = clock::now();
    while (!glfwWindowShouldClose(m_Window)) {
        auto now = clock::now();
        float dt = std::chrono::duration<float>(now - last).count();
        last = now;

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glClearColor(0.5f,0.7f,1.0f,1.0f);

        if (m_Scene) {
            m_Scene->OnUpdate(dt);
            m_Scene->OnRender();
        }

        glfwSwapBuffers(m_Window);
        glfwPollEvents();
    }
}
