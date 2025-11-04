#include "../../include/objects/MovableObject.h"
#include "../../include/terrain.h"
#include <GL/glew.h>
#include <cmath>
#include "../../include/objects.h"
#include <algorithm>

MovableObject::MovableObject(float x, float y, float z)
    : position(x, y, z), yaw(0.0f) {
    updateVectors();
    stayOnTerrain();
}

void MovableObject::MoveForward(float amount) {
    // Forward already reflects current yaw (updated in Turn)
    glm::vec3 proposed = position + forward * amount;
    // Keep movement confined to XZ; radius of player for collision
    const float radius = 0.5f; // matches visual size (half-box + margin)
    // Terrain bounds (match drawTerrain grid: SIZE=256, SPACING=0.5)
    const int TERRAIN_SIZE = 512;
    const float SPACING = 0.5f;
    const float minX = -((TERRAIN_SIZE/2) * SPACING);
    const float minZ = -((TERRAIN_SIZE/2) * SPACING);
    const float maxX = ((TERRAIN_SIZE/2) - 1) * SPACING;
    const float maxZ = ((TERRAIN_SIZE/2) - 1) * SPACING;
    // Clamp to terrain bounds
    proposed.x = std::max(minX, std::min(maxX, proposed.x));
    proposed.z = std::max(minZ, std::min(maxZ, proposed.z));
    // Check building collision; if not colliding, accept move
    if (!isPositionInsideBuilding(proposed.x, proposed.z, radius)) {
        position = proposed;
        stayOnTerrain();
    }
}

void MovableObject::MoveRight(float amount) {
    glm::vec3 proposed = position + right * amount;
    const float radius = 0.5f;
    const int TERRAIN_SIZE = 512;
    const float SPACING = 0.5f;
    const float minX = -((TERRAIN_SIZE/2) * SPACING);
    const float minZ = -((TERRAIN_SIZE/2) * SPACING);
    const float maxX = ((TERRAIN_SIZE/2) - 1) * SPACING;
    const float maxZ = ((TERRAIN_SIZE/2) - 1) * SPACING;
    proposed.x = std::max(minX, std::min(maxX, proposed.x));
    proposed.z = std::max(minZ, std::min(maxZ, proposed.z));
    if (!isPositionInsideBuilding(proposed.x, proposed.z, radius)) {
        position = proposed;
        stayOnTerrain();
    }
}

void MovableObject::MoveInDirection(const glm::vec3& direction, float amount) {
    glm::vec3 normalizedDir = glm::normalize(direction);
    // Only move on XZ plane (ignore Y component of direction to stay on terrain)
    glm::vec3 moveDir = glm::vec3(normalizedDir.x, 0.0f, normalizedDir.z);
    glm::vec3 proposed = position + moveDir * amount;
    const float radius = 0.5f;
    const int TERRAIN_SIZE = 512;
    const float SPACING = 0.5f;
    const float minX = -((TERRAIN_SIZE/2) * SPACING);
    const float minZ = -((TERRAIN_SIZE/2) * SPACING);
    const float maxX = ((TERRAIN_SIZE/2) - 1) * SPACING;
    const float maxZ = ((TERRAIN_SIZE/2) - 1) * SPACING;
    proposed.x = std::max(minX, std::min(maxX, proposed.x));
    proposed.z = std::max(minZ, std::min(maxZ, proposed.z));
    if (!isPositionInsideBuilding(proposed.x, proposed.z, radius)) {
        position = proposed;
        stayOnTerrain();
    }
}

void MovableObject::Draw() const {
    glPushMatrix();
    
    // Move to object position
    glTranslatef(position.x, position.y, position.z);
    glRotatef(yaw, 0.0f, 1.0f, 0.0f);
    
    // Draw a simple box for now
    glColor3f(1.0f, 0.0f, 0.0f); // Red color
    
    float size = 0.5f;
    glBegin(GL_QUADS);
    
    // Front face
    glVertex3f(-size, -size, size);
    glVertex3f(size, -size, size);
    glVertex3f(size, size, size);
    glVertex3f(-size, size, size);
    
    // Back face
    glVertex3f(-size, -size, -size);
    glVertex3f(-size, size, -size);
    glVertex3f(size, size, -size);
    glVertex3f(size, -size, -size);
    
    // Top face
    glVertex3f(-size, size, -size);
    glVertex3f(-size, size, size);
    glVertex3f(size, size, size);
    glVertex3f(size, size, -size);
    
    // Bottom face
    glVertex3f(-size, -size, -size);
    glVertex3f(size, -size, -size);
    glVertex3f(size, -size, size);
    glVertex3f(-size, -size, size);
    
    // Right face
    glVertex3f(size, -size, -size);
    glVertex3f(size, size, -size);
    glVertex3f(size, size, size);
    glVertex3f(size, -size, size);
    
    // Left face
    glVertex3f(-size, -size, -size);
    glVertex3f(-size, -size, size);
    glVertex3f(-size, size, size);
    glVertex3f(-size, size, -size);
    
    glEnd();
    
    glPopMatrix();
}

void MovableObject::updateVectors() {
    // Calculate forward and right vectors from yaw angle
    float radians = glm::radians(yaw);
    forward = glm::normalize(glm::vec3(std::sin(radians), 0.0f, -std::cos(radians)));
    right   = glm::normalize(glm::vec3(std::cos(radians), 0.0f,  std::sin(radians)));
}

void MovableObject::stayOnTerrain() {
    // Update Y position to stay on terrain surface
    position.y = getTerrainHeight(position.x, position.z) + 0.5f; // Add 0.5 to float above terrain
}