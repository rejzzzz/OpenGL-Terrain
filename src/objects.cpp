#include <GL/glew.h>
#include "../include/terrain.h"

// Helper: draw a simple axis-aligned cube centered at origin
void drawCube(float width, float height, float depth) {
    float w2 = width * 0.5f;
    float h2 = height * 0.5f;
    float d2 = depth * 0.5f;

    glBegin(GL_QUADS);
    // Front face
    glColor3f(0.6f, 0.6f, 0.6f);
    glVertex3f(-w2, -h2,  d2);
    glVertex3f( w2, -h2,  d2);
    glVertex3f( w2,  h2,  d2);
    glVertex3f(-w2,  h2,  d2);
    // Back face
    glVertex3f(-w2, -h2, -d2);
    glVertex3f(-w2,  h2, -d2);
    glVertex3f( w2,  h2, -d2);
    glVertex3f( w2, -h2, -d2);
    // Top face
    glColor3f(0.7f, 0.7f, 0.7f);
    glVertex3f(-w2,  h2, -d2);
    glVertex3f(-w2,  h2,  d2);
    glVertex3f( w2,  h2,  d2);
    glVertex3f( w2,  h2, -d2);
    // Bottom face
    glColor3f(0.5f, 0.5f, 0.5f);
    glVertex3f(-w2, -h2, -d2);
    glVertex3f( w2, -h2, -d2);
    glVertex3f( w2, -h2,  d2);
    glVertex3f(-w2, -h2,  d2);
    // Right face
    glColor3f(0.65f, 0.65f, 0.65f);
    glVertex3f( w2, -h2, -d2);
    glVertex3f( w2,  h2, -d2);
    glVertex3f( w2,  h2,  d2);
    glVertex3f( w2, -h2,  d2);
    // Left face
    glVertex3f(-w2, -h2, -d2);
    glVertex3f(-w2, -h2,  d2);
    glVertex3f(-w2,  h2,  d2);
    glVertex3f(-w2,  h2, -d2);
    glEnd();
}

// Draw a simple tree using cubes only (no GLUT)
void drawTree(float x, float z) {
    float y = getTerrainHeight(x, z);
    glPushMatrix();
    glTranslatef(x, y, z);

    // Trunk: small brown cube
    glColor3f(0.35f, 0.2f, 0.1f); // Brown
    drawCube(0.3f, 1.0f, 0.3f);

    // Foliage: green cube on top (crude, but works)
    glTranslatef(0.0f, 1.0f, 0.0f); // Move up
    glColor3f(0.0f, 0.7f, 0.2f);   // Green
    drawCube(1.2f, 1.2f, 1.2f);

    glPopMatrix();
}

void drawTrees() {
    drawTree(5.0f, 5.0f);
    drawTree(-6.0f, 3.0f);
    drawTree(2.0f, -7.0f);
}

void drawBuildings() {
    float x = -4.0f, z = -4.0f;
    float y = getTerrainHeight(x, z);
    glPushMatrix();
    glTranslatef(x, y, z);
    drawCube(2.0f, 3.0f, 2.0f); // building
    glPopMatrix();
}