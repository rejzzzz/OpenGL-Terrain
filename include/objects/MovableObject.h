#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <cmath>

// Represents a movable object in the terrain scene
class MovableObject {
public:
    MovableObject(float x, float y, float z);

    // Move object on the XZ plane, Y coord will be set based on terrain height
    void MoveForward(float amount);
    void MoveRight(float amount);

    // Move in a specified direction (normalized vector), amount is distance
    void MoveInDirection(const glm::vec3& direction, float amount);

    // New movement API: set desired movement each frame (direction on XZ plane, speed in world units/sec)
    void SetDesiredMovement(const glm::vec3 &dir, float speed);
    // Update physics / smoothing / animations (call each frame with dt seconds)
    void Update(float dt);
    
    // Draw the object
    void Draw();
    
    // Get current position for camera tracking
    const glm::vec3& GetPosition() const { return position; }
    float GetYaw() const { return yaw; }
    void SetYaw(float newYaw) { yaw = newYaw; updateVectors(); }

    // Turn the object left/right (updates direction vectors so controls stay intuitive)
    void Turn(float degrees) {
        yaw += degrees;
        // Keep yaw within a reasonable range to avoid float drift
        if (yaw > 360.0f || yaw < -360.0f) yaw = std::fmod(yaw, 360.0f);
        updateVectors();
    }

private:
    glm::vec3 position;  // Current world position
    glm::vec3 forward;   // Forward direction vector
    glm::vec3 right;     // Right direction vector
    float yaw;           // Current rotation around Y axis
    
    void updateVectors();
    void stayOnTerrain();

    // Movement smoothing
    glm::vec3 velocity = glm::vec3(0.0f);
    glm::vec3 desiredVelocity = glm::vec3(0.0f);

    // Animation state
    float animPhase = 0.0f; // advances when moving
    float animSpeed = 6.0f;
};