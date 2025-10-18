#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>       // <-- ADD THIS
#include <glm/gtc/matrix_transform.hpp> // for radians(), lookAt(), etc.
#include <iostream>
#include <cmath>


// Window dimensions
const int WIDTH = 800;
const int HEIGHT = 600;

// Rotation and movement variables
float autoRotY = 0.0f;
float cubePosX = 0.0f, cubePosZ = 0.0f;
int currentTarget = 0;

// Hexagon vertices (XZ plane)
const int NUM_POINTS = 6;
float hexagon[NUM_POINTS][2];

// Camera orbit variables
float yaw = 0.0f;        // Horizontal rotation
float pitch = 20.0f;     // Vertical rotation
float distance = 10.0f;  // Distance from center
bool rightMousePressed = false;
double lastX, lastY;

// Generate hexagon points
void generateHexagon(float radius = 4.0f) {
    for (int i = 0; i < NUM_POINTS; i++) {
        float angle = M_PI / 3 * i;
        hexagon[i][0] = radius * cos(angle);
        hexagon[i][1] = radius * sin(angle);
    }
}

// Keyboard controls
void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    if (key == GLFW_KEY_LEFT && action == GLFW_PRESS) {
        currentTarget = (currentTarget - 1 + NUM_POINTS) % NUM_POINTS;
    }

    if (key == GLFW_KEY_RIGHT && action == GLFW_PRESS) {
        currentTarget = (currentTarget + 1) % NUM_POINTS;
    }
}

// Mouse button callback
void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods) {
    if (button == GLFW_MOUSE_BUTTON_RIGHT) {
        if (action == GLFW_PRESS) {
            rightMousePressed = true;
            glfwGetCursorPos(window, &lastX, &lastY);
        } else if (action == GLFW_RELEASE) {
            rightMousePressed = false;
        }
    }
}

// Mouse movement callback (for camera)
void mouseCallback(GLFWwindow* window, double xpos, double ypos) {
    if (!rightMousePressed) return;

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos; // reversed y
    lastX = xpos;
    lastY = ypos;

    float sensitivity = 0.2f;
    xoffset *= sensitivity;
    yoffset *= sensitivity;

    yaw += xoffset;
    pitch += yoffset;

    if (pitch > 89.0f) pitch = 89.0f;
    if (pitch < -89.0f) pitch = -89.0f;
}

// Draw cube (same as before)
void drawCube() {
    glBegin(GL_QUADS);
    
    // Front face (red)
    glColor3f(1.0f, 0.0f, 0.0f);
    glVertex3f(-1.0f, -1.0f,  1.0f);
    glVertex3f( 1.0f, -1.0f,  1.0f);
    glVertex3f( 1.0f,  1.0f,  1.0f);
    glVertex3f(-1.0f,  1.0f,  1.0f);
    
    // Back face (green)
    glColor3f(0.0f, 1.0f, 0.0f);
    glVertex3f(-1.0f, -1.0f, -1.0f);
    glVertex3f(-1.0f,  1.0f, -1.0f);
    glVertex3f( 1.0f,  1.0f, -1.0f);
    glVertex3f( 1.0f, -1.0f, -1.0f);
    
    // Top face (blue)
    glColor3f(0.0f, 0.0f, 1.0f);
    glVertex3f(-1.0f,  1.0f, -1.0f);
    glVertex3f(-1.0f,  1.0f,  1.0f);
    glVertex3f( 1.0f,  1.0f,  1.0f);
    glVertex3f( 1.0f,  1.0f, -1.0f);
    
    // Bottom face (yellow)
    glColor3f(1.0f, 1.0f, 0.0f);
    glVertex3f(-1.0f, -1.0f, -1.0f);
    glVertex3f( 1.0f, -1.0f, -1.0f);
    glVertex3f( 1.0f, -1.0f,  1.0f);
    glVertex3f(-1.0f, -1.0f,  1.0f);
    
    // Right face (magenta)
    glColor3f(1.0f, 0.0f, 1.0f);
    glVertex3f( 1.0f, -1.0f, -1.0f);
    glVertex3f( 1.0f,  1.0f, -1.0f);
    glVertex3f( 1.0f,  1.0f,  1.0f);
    glVertex3f( 1.0f, -1.0f,  1.0f);
    
    // Left face (cyan)
    glColor3f(0.0f, 1.0f, 1.0f);
    glVertex3f(-1.0f, -1.0f, -1.0f);
    glVertex3f(-1.0f, -1.0f,  1.0f);
    glVertex3f(-1.0f,  1.0f,  1.0f);
    glVertex3f(-1.0f,  1.0f, -1.0f);
    
    glEnd();
}

// Manual gluLookAt equivalent
void setCamera(float eyeX, float eyeY, float eyeZ,
               float centerX, float centerY, float centerZ,
               float upX, float upY, float upZ) {
    float forward[3] = { centerX - eyeX, centerY - eyeY, centerZ - eyeZ };
    float up[3] = { upX, upY, upZ };
    float fMag = sqrt(forward[0]*forward[0] + forward[1]*forward[1] + forward[2]*forward[2]);
    forward[0] /= fMag; forward[1] /= fMag; forward[2] /= fMag;

    float s[3] = {
        forward[1]*up[2] - forward[2]*up[1],
        forward[2]*up[0] - forward[0]*up[2],
        forward[0]*up[1] - forward[1]*up[0]
    };
    float sMag = sqrt(s[0]*s[0] + s[1]*s[1] + s[2]*s[2]);
    s[0]/=sMag; s[1]/=sMag; s[2]/=sMag;

    float u[3] = {
        s[1]*forward[2] - s[2]*forward[1],
        s[2]*forward[0] - s[0]*forward[2],
        s[0]*forward[1] - s[1]*forward[0]
    };

    float m[16] = {
        s[0],  u[0], -forward[0], 0,
        s[1],  u[1], -forward[1], 0,
        s[2],  u[2], -forward[2], 0,
        0,     0,     0,          1
    };
    glMultMatrixf(m);
    glTranslatef(-eyeX, -eyeY, -eyeZ);
}

int main() {
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW\n";
        return -1;
    }

    GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "Hexagon Cube with Camera", NULL, NULL);
    if (!window) {
        std::cerr << "Failed to create GLFW window\n";
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);
    glewInit();

    glfwSetKeyCallback(window, keyCallback);
    glfwSetCursorPosCallback(window, mouseCallback);
    glfwSetMouseButtonCallback(window, mouseButtonCallback);

    glEnable(GL_DEPTH_TEST);

    // Perspective setup
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    float fov = 45.0f, aspect = (float)WIDTH / HEIGHT, near = 0.1f, far = 100.0f;
    float fH = tan(fov / 360.0f * M_PI) * near;
    float fW = fH * aspect;
    glFrustum(-fW, fW, -fH, fH, near, far);
    glMatrixMode(GL_MODELVIEW);

    generateHexagon(4.0f);

    std::cout << "Controls:\n- Left/Right: move along hex path\n- Right Mouse Drag: rotate camera\n- ESC: exit\n";

    while (!glfwWindowShouldClose(window)) {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glClearColor(0.15f, 0.2f, 0.25f, 1.0f);

        glLoadIdentity();

        // Camera orbit around cube
        float camX = distance * cos(glm::radians(pitch)) * sin(glm::radians(yaw));
        float camY = distance * sin(glm::radians(pitch));
        float camZ = distance * cos(glm::radians(pitch)) * cos(glm::radians(yaw));

        setCamera(camX, camY, camZ, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f);

        // Smoothly move cube toward current vertex
        float targetX = hexagon[currentTarget][0];
        float targetZ = hexagon[currentTarget][1];
        cubePosX += (targetX - cubePosX) * 0.02f;
        cubePosZ += (targetZ - cubePosZ) * 0.02f;

        // Draw cube
        glPushMatrix();
        glTranslatef(cubePosX, 0.0f, cubePosZ);

        // Slower auto rotation
        autoRotY += 0.1f;
        if (autoRotY > 360.0f) autoRotY -= 360.0f;
        glRotatef(autoRotY, 0.0f, 1.0f, 0.0f);

        drawCube();
        glPopMatrix();

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}
