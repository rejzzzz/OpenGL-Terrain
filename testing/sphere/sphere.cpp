#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <cmath>
#include <vector>

// Window dimensions
const int WIDTH = 800;
const int HEIGHT = 600;

// Sphere parameters
float radius = 1.0f;
int slices = 30;
int stacks = 30;
bool wireframe = false;
bool showNormals = false;

// Interaction variables
float rotX = 0.0f;
float rotY = 0.0f;
float zoom = -5.0f;
float lastX = WIDTH / 2.0f;
float lastY = HEIGHT / 2.0f;
bool firstMouse = true;
bool mousePressed = false;

// Animation
bool autoRotate = false;
float autoRotateSpeed = 1.0f;

// Color mode
int colorMode = 0; // 0: solid, 1: gradient, 2: rainbow

// Mouse callback function
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

    float sensitivity = 0.5f;
    xOffset *= sensitivity;
    yOffset *= sensitivity;

    rotY += xOffset;
    rotX += yOffset;

    // Constrain pitch
    if (rotX > 89.0f) rotX = 89.0f;
    if (rotX < -89.0f) rotX = -89.0f;
}

// Mouse button callback
void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods) {
    if (button == GLFW_MOUSE_BUTTON_LEFT) {
        if (action == GLFW_PRESS) {
            mousePressed = true;
            firstMouse = true;
        } else if (action == GLFW_RELEASE) {
            mousePressed = false;
        }
    }
}

// Scroll callback for zooming
void scrollCallback(GLFWwindow* window, double xoffset, double yoffset) {
    zoom += yoffset * 0.5f;
    if (zoom > -1.0f) zoom = -1.0f;
    if (zoom < -20.0f) zoom = -20.0f;
}

// Key callback for controls
void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (action == GLFW_PRESS || action == GLFW_REPEAT) {
        switch (key) {
            case GLFW_KEY_ESCAPE:
                glfwSetWindowShouldClose(window, true);
                break;
            case GLFW_KEY_R:
                // Reset rotation and zoom
                rotX = 0.0f;
                rotY = 0.0f;
                zoom = -5.0f;
                break;
            case GLFW_KEY_W:
                // Toggle wireframe
                wireframe = !wireframe;
                break;
            case GLFW_KEY_N:
                // Toggle normals display
                showNormals = !showNormals;
                break;
            case GLFW_KEY_SPACE:
                // Toggle auto rotation
                autoRotate = !autoRotate;
                break;
            case GLFW_KEY_C:
                // Cycle color modes
                colorMode = (colorMode + 1) % 3;
                break;
            case GLFW_KEY_UP:
                // Increase detail
                if (slices < 100) {
                    slices += 5;
                    stacks += 5;
                }
                break;
            case GLFW_KEY_DOWN:
                // Decrease detail
                if (slices > 10) {
                    slices -= 5;
                    stacks -= 5;
                }
                break;
            case GLFW_KEY_EQUAL: // '+' key
                // Increase size
                radius += 0.1f;
                if (radius > 3.0f) radius = 3.0f;
                break;
            case GLFW_KEY_MINUS:
                // Decrease size
                radius -= 0.1f;
                if (radius < 0.2f) radius = 0.2f;
                break;
            case GLFW_KEY_LEFT:
                // Decrease rotation speed
                autoRotateSpeed -= 0.5f;
                if (autoRotateSpeed < 0.1f) autoRotateSpeed = 0.1f;
                break;
            case GLFW_KEY_RIGHT:
                // Increase rotation speed
                autoRotateSpeed += 0.5f;
                if (autoRotateSpeed > 5.0f) autoRotateSpeed = 5.0f;
                break;
        }
    }
}

void setColor(int mode, float u, float v) {
    switch (mode) {
        case 0: // Solid blue
            glColor3f(0.3f, 0.5f, 1.0f);
            break;
        case 1: // Gradient from blue to red
            glColor3f(v, 0.3f, 1.0f - v);
            break;
        case 2: // Rainbow based on position
            {
                float hue = u * 6.0f; // 0 to 6
                float c = 1.0f;
                float x = c * (1.0f - fabs(fmod(hue, 2.0f) - 1.0f));
                
                if (hue < 1.0f) glColor3f(c, x, 0);
                else if (hue < 2.0f) glColor3f(x, c, 0);
                else if (hue < 3.0f) glColor3f(0, c, x);
                else if (hue < 4.0f) glColor3f(0, x, c);
                else if (hue < 5.0f) glColor3f(x, 0, c);
                else glColor3f(c, 0, x);
            }
            break;
    }
}

void drawSphere() {
    for (int i = 0; i < stacks; i++) {
        float lat0 = M_PI * (-0.5f + (float)i / stacks);
        float z0 = sin(lat0);
        float zr0 = cos(lat0);

        float lat1 = M_PI * (-0.5f + (float)(i + 1) / stacks);
        float z1 = sin(lat1);
        float zr1 = cos(lat1);

        if (wireframe) {
            glBegin(GL_LINES);
        } else {
            glBegin(GL_QUAD_STRIP);
        }

        for (int j = 0; j <= slices; j++) {
            float lng = 2 * M_PI * (float)j / slices;
            float x = cos(lng);
            float y = sin(lng);

            // First vertex
            float u1 = (float)j / slices;
            float v1 = (float)i / stacks;
            setColor(colorMode, u1, v1);
            
            glNormal3f(x * zr0, y * zr0, z0);
            glVertex3f(radius * x * zr0, radius * y * zr0, radius * z0);

            // Second vertex
            float v2 = (float)(i + 1) / stacks;
            setColor(colorMode, u1, v2);
            
            glNormal3f(x * zr1, y * zr1, z1);
            glVertex3f(radius * x * zr1, radius * y * zr1, radius * z1);
        }
        glEnd();
    }
}

void drawNormals() {
    if (!showNormals) return;
    
    glColor3f(1.0f, 1.0f, 0.0f); // Yellow normals
    glBegin(GL_LINES);
    
    for (int i = 0; i < stacks; i += 3) { // Skip some for clarity
        for (int j = 0; j < slices; j += 3) {
            float lat = M_PI * (-0.5f + (float)i / stacks);
            float lng = 2 * M_PI * (float)j / slices;
            
            float z = sin(lat);
            float zr = cos(lat);
            float x = cos(lng) * zr;
            float y = sin(lng) * zr;
            
            // Point on sphere
            float px = radius * x;
            float py = radius * y;
            float pz = radius * z;
            
            // Normal (same as position for unit sphere)
            float normalLength = 0.3f;
            
            glVertex3f(px, py, pz);
            glVertex3f(px + x * normalLength, py + y * normalLength, pz + z * normalLength);
        }
    }
    glEnd();
}

void printControls() {
    std::cout << "\n=== Interactive Sphere Controls ===" << std::endl;
    std::cout << "Mouse:" << std::endl;
    std::cout << "  Left click + drag: Rotate sphere" << std::endl;
    std::cout << "  Scroll wheel: Zoom in/out" << std::endl;
    std::cout << "\nKeyboard:" << std::endl;
    std::cout << "  SPACE: Toggle auto-rotation" << std::endl;
    std::cout << "  W: Toggle wireframe mode" << std::endl;
    std::cout << "  N: Toggle normal vectors display" << std::endl;
    std::cout << "  C: Cycle color modes (solid/gradient/rainbow)" << std::endl;
    std::cout << "  UP/DOWN: Increase/decrease sphere detail" << std::endl;
    std::cout << "  +/-: Increase/decrease sphere size" << std::endl;
    std::cout << "  LEFT/RIGHT: Adjust auto-rotation speed" << std::endl;
    std::cout << "  R: Reset rotation and zoom" << std::endl;
    std::cout << "  ESC: Exit" << std::endl;
    std::cout << "==================================\n" << std::endl;
}

int main() {
    // Initialize GLFW
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        return -1;
    }

    // Create window
    GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "Interactive Sphere", NULL, NULL);
    if (!window) {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);

    // Initialize GLEW
    if (glewInit() != GLEW_OK) {
        std::cerr << "Failed to initialize GLEW" << std::endl;
        return -1;
    }

    // Set callbacks
    glfwSetCursorPosCallback(window, mouseCallback);
    glfwSetMouseButtonCallback(window, mouseButtonCallback);
    glfwSetScrollCallback(window, scrollCallback);
    glfwSetKeyCallback(window, keyCallback);

    // Enable depth testing and lighting
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    
    // Set up lighting
    float lightPos[] = {2.0f, 2.0f, 2.0f, 1.0f};
    float lightColor[] = {1.0f, 1.0f, 1.0f, 1.0f};
    float ambientLight[] = {0.2f, 0.2f, 0.2f, 1.0f};
    
    glLightfv(GL_LIGHT0, GL_POSITION, lightPos);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, lightColor);
    glLightfv(GL_LIGHT0, GL_AMBIENT, ambientLight);

    // Enable color material
    glEnable(GL_COLOR_MATERIAL);
    glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
    
    // Set up perspective projection
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    
    float fov = 45.0f * M_PI / 180.0f;
    float aspect = (float)WIDTH / (float)HEIGHT;
    float near = 0.1f;
    float far = 100.0f;
    float fH = tan(fov / 2.0f) * near;
    float fW = fH * aspect;
    glFrustum(-fW, fW, -fH, fH, near, far);
    
    glViewport(0, 0, WIDTH, HEIGHT);
    glMatrixMode(GL_MODELVIEW);

    printControls();

    // Main render loop
    while (!glfwWindowShouldClose(window)) {
        // Auto rotation
        if (autoRotate) {
            rotY += autoRotateSpeed;
            if (rotY > 360.0f) rotY -= 360.0f;
        }

        // Clear the screen
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glClearColor(0.1f, 0.1f, 0.2f, 1.0f);

        // Set up model view matrix
        glLoadIdentity();
        glTranslatef(0.0f, 0.0f, zoom);
        
        // Apply rotations
        glRotatef(rotX, 1.0f, 0.0f, 0.0f);
        glRotatef(rotY, 0.0f, 1.0f, 0.0f);

        // Draw the sphere
        if (wireframe) {
            glDisable(GL_LIGHTING);
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        } else {
            glEnable(GL_LIGHTING);
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        }
        
        drawSphere();
        
        // Draw normals if enabled
        glDisable(GL_LIGHTING);
        drawNormals();

        // Swap buffers and poll events
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // Cleanup
    glfwTerminate();
    return 0;
}