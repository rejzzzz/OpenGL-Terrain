#include "BuildingObject.h"
#include <glm/gtc/matrix_transform.hpp>
#include <glad/glad.h> // or GLEW depending on your project
#include <vector>

unsigned int BuildingObject::cubeVAO = 0;
unsigned int BuildingObject::cubeVBO = 0;

void BuildingObject::initCube() {
    if (cubeVAO) return; // already initialized

    float vertices[] = {
        // positions         // normals (optional) // texcoords (optional)
        // back face
        -0.5f, 0.0f, -0.5f,  0.0f, 0.0f, -1.0f,
         0.5f, 1.0f, -0.5f,  0.0f, 0.0f, -1.0f,
         0.5f, 0.0f, -0.5f,  0.0f, 0.0f, -1.0f,
         0.5f, 1.0f, -0.5f,  0.0f, 0.0f, -1.0f,
        -0.5f, 0.0f, -0.5f,  0.0f, 0.0f, -1.0f,
        -0.5f, 1.0f, -0.5f,  0.0f, 0.0f, -1.0f,

        // (repeat for other 5 faces) -- to keep snippet short you can use a cube generator
        // For simplicity below we'll use a quicker cube (36 vertices). See common cube data online.
    };

    // For brevity: use a simple 36-vertex cube array (positions only).
    float simpleCube[] = {
        // positions
        -0.5f, 0.0f, -0.5f,  0.5f, 1.0f, -0.5f,   0.5f, 0.0f, -0.5f,
        0.5f, 1.0f, -0.5f,   -0.5f, 0.0f, -0.5f,  -0.5f, 1.0f, -0.5f,
        // ... full 36-vertex cube data needed here
    };

    // A robust approach: create a unit cube using 36 vertices (search "36 vertex cube array")
    // For this snippet, we will fill with a standard cube array (full data below):

    float cubeVertices[] = {
        // positions
        -0.5f, 0.0f, -0.5f,
         0.5f, 1.0f, -0.5f,
         0.5f, 0.0f, -0.5f,
         0.5f, 1.0f, -0.5f,
        -0.5f, 0.0f, -0.5f,
        -0.5f, 1.0f, -0.5f,

        -0.5f, 0.0f,  0.5f,
         0.5f, 0.0f,  0.5f,
         0.5f, 1.0f,  0.5f,
         0.5f, 1.0f,  0.5f,
        -0.5f, 1.0f,  0.5f,
        -0.5f, 0.0f,  0.5f,

        -0.5f, 1.0f,  0.5f,
        -0.5f, 1.0f, -0.5f,
        -0.5f, 0.0f, -0.5f,
        -0.5f, 0.0f, -0.5f,
        -0.5f, 0.0f,  0.5f,
        -0.5f, 1.0f,  0.5f,

         0.5f, 1.0f,  0.5f,
         0.5f, 0.0f,  0.5f,
         0.5f, 0.0f, -0.5f,
         0.5f, 0.0f, -0.5f,
         0.5f, 1.0f, -0.5f,
         0.5f, 1.0f,  0.5f,

        -0.5f, 0.0f, -0.5f,
         0.5f, 0.0f, -0.5f,
         0.5f, 0.0f,  0.5f,
         0.5f, 0.0f,  0.5f,
        -0.5f, 0.0f,  0.5f,
        -0.5f, 0.0f, -0.5f,

        -0.5f, 1.0f, -0.5f,
        -0.5f, 1.0f,  0.5f,
         0.5f, 1.0f,  0.5f,
         0.5f, 1.0f,  0.5f,
         0.5f, 1.0f, -0.5f,
        -0.5f, 1.0f, -0.5f
    };

    glGenVertexArrays(1, &cubeVAO);
    glGenBuffers(1, &cubeVBO);
    glBindVertexArray(cubeVAO);
    glBindBuffer(GL_ARRAY_BUFFER, cubeVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(cubeVertices), cubeVertices, GL_STATIC_DRAW);
    // position attribute
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glBindVertexArray(0);
}

void BuildingObject::draw(Shader &shader) {
    if (!cubeVAO) initCube();

    shader.use();
    glm::mat4 model = glm::mat4(1.0f);
    // shift cube so base sits on terrain (we used 0.0 -> 1.0 height in cube)
    model = glm::translate(model, position);
    model = glm::scale(model, glm::vec3(scale, scale, scale));
    shader.setMat4("model", model);

    glBindVertexArray(cubeVAO);
    glDrawArrays(GL_TRIANGLES, 0, 36);
    glBindVertexArray(0);
}
