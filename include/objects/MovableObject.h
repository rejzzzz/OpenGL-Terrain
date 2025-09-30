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
    
    // Draw the object
    void Draw() const;
    
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
};