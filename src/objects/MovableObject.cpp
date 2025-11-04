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
    const float radius = 0.6f; // slightly larger to prevent entering thin building edges
    // Terrain bounds (match drawTerrain grid: SIZE=256, SPACING=0.5)
    const int TERRAIN_SIZE = 200;
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
    const float radius = 0.6f;
    const int TERRAIN_SIZE = 200;
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
    const float radius = 0.6f;
    const int TERRAIN_SIZE = 200;
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

void MovableObject::Draw() {
    glPushMatrix();
    
    // Move to object position
    glTranslatef(position.x, position.y, position.z);
    glRotatef(yaw, 0.0f, 1.0f, 0.0f);
    
    // Draw a simple humanoid built from boxes: legs, torso, arms, head
    // The object's position is the same convention as before: center offset such that
    // the bottom of the model aligns with terrain (previously cube of size 0.5 had bottom at -0.5)
    // We'll keep that baseline: feet start at y = -0.5

    // helper to draw an axis-aligned box centered at origin with given extents
    auto drawBox = [](float hw, float hh, float hd){
        float x0 = -hw, x1 = hw;
        float y0 = -hh, y1 = hh;
        float z0 = -hd, z1 = hd;
        glBegin(GL_QUADS);
        // front
        glVertex3f(x0, y0, z1);
        glVertex3f(x1, y0, z1);
        glVertex3f(x1, y1, z1);
        glVertex3f(x0, y1, z1);
        // back
        glVertex3f(x0, y0, z0);
        glVertex3f(x0, y1, z0);
        glVertex3f(x1, y1, z0);
        glVertex3f(x1, y0, z0);
        // top
        glVertex3f(x0, y1, z0);
        glVertex3f(x0, y1, z1);
        glVertex3f(x1, y1, z1);
        glVertex3f(x1, y1, z0);
        // bottom
        glVertex3f(x0, y0, z0);
        glVertex3f(x1, y0, z0);
        glVertex3f(x1, y0, z1);
        glVertex3f(x0, y0, z1);
        // right
        glVertex3f(x1, y0, z0);
        glVertex3f(x1, y1, z0);
        glVertex3f(x1, y1, z1);
        glVertex3f(x1, y0, z1);
        // left
        glVertex3f(x0, y0, z0);
        glVertex3f(x0, y0, z1);
        glVertex3f(x0, y1, z1);
        glVertex3f(x0, y1, z0);
        glEnd();
    };

    // model dimensions
    const float footBaseY = -0.5f; // bottom of feet
    const float legHeight = 0.6f;
    const float legHalfW = 0.12f, legHalfD = 0.12f;
    const float torsoHeight = 0.8f;
    const float torsoHalfW = 0.28f, torsoHalfD = 0.18f;
    const float armHalfW = 0.10f, armHalfD = 0.10f, armHeight = 0.6f;
    const float headHalf = 0.18f;

    // Legs (two) with simple walking swing based on animPhase
    float speedFactor = glm::length(glm::vec2(velocity.x, velocity.z));
    float swing = std::sin(animPhase) * 30.0f * speedFactor; // degrees
    glColor3f(0.15f, 0.15f, 0.45f); // pants: dark bluish
    // left leg (swing opposite)
    glPushMatrix();
    glTranslatef(-0.18f, footBaseY + legHeight*0.5f, 0.0f);
    glRotatef(-swing, 1.0f, 0.0f, 0.0f);
    drawBox(legHalfW, legHeight*0.5f, legHalfD);
    glPopMatrix();
    // right leg
    glPushMatrix();
    glTranslatef(0.18f, footBaseY + legHeight*0.5f, 0.0f);
    glRotatef(swing, 1.0f, 0.0f, 0.0f);
    drawBox(legHalfW, legHeight*0.5f, legHalfD);
    glPopMatrix();

    // Torso
    glColor3f(0.8f, 0.35f, 0.2f); // shirt: brownish
    glPushMatrix();
    float torsoY = footBaseY + legHeight + torsoHeight*0.5f;
    glTranslatef(0.0f, torsoY, 0.0f);
    drawBox(torsoHalfW, torsoHeight*0.5f, torsoHalfD);
    glPopMatrix();

    // Arms - swing opposite to legs
    float armSwing = std::sin(animPhase) * 25.0f * speedFactor;
    glColor3f(0.9f, 0.8f, 0.6f); // skin tone
    // left arm
    glPushMatrix();
    glTranslatef(-torsoHalfW - armHalfW - 0.02f, torsoY, 0.0f);
    glRotatef(armSwing, 1.0f, 0.0f, 0.0f);
    drawBox(armHalfW, armHeight*0.5f, armHalfD);
    glPopMatrix();
    // right arm
    glPushMatrix();
    glTranslatef(torsoHalfW + armHalfW + 0.02f, torsoY, 0.0f);
    glRotatef(-armSwing, 1.0f, 0.0f, 0.0f);
    drawBox(armHalfW, armHeight*0.5f, armHalfD);
    glPopMatrix();

    // Head
    glColor3f(0.95f, 0.85f, 0.7f); // skin
    glPushMatrix();
    float headY = torsoY + torsoHeight*0.5f + headHalf;
    glTranslatef(0.0f, headY, 0.0f);
    drawBox(headHalf, headHalf, headHalf);
    glPopMatrix();
    
    glPopMatrix();
}

void MovableObject::SetDesiredMovement(const glm::vec3 &dir, float speed) {
    glm::vec3 d = dir;
    d.y = 0.0f;
    if (glm::length(d) > 0.0001f) d = glm::normalize(d) * speed;
    else d = glm::vec3(0.0f);
    desiredVelocity = d;
}

void MovableObject::Update(float dt) {
    // Smoothly approach desired velocity
    const float smooth = 10.0f; // larger -> quicker responsiveness
    float alpha = 1.0f - std::exp(-smooth * dt);
    velocity = glm::mix(velocity, desiredVelocity, alpha);

    // Attempt to move by velocity*dt, with collision check
    glm::vec3 proposed = position + velocity * dt;
    const float radius = 0.6f;
    const int TERRAIN_SIZE = 200;
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
    } else {
        // stop on collision
        velocity = glm::vec3(0.0f);
    }

    // advance animation phase depending on speed
    float sp = glm::length(glm::vec2(velocity.x, velocity.z));
    animPhase += dt * animSpeed * (0.5f + sp * 2.0f);
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