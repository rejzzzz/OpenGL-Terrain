#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

class MovableObject; // forward declare

// Third-person orbit camera following a MovableObject target.
class Camera {
public:
    explicit Camera(const MovableObject* target = nullptr);

    void SetTarget(const MovableObject* target) { m_Target = target; }
    void SetYOffset(float y) { m_TargetYOffset = y; }

    void ProcessMouseMovement(float dx, float dy); // dx, dy in pixels
    void ProcessScroll(float dy);                  // scroll delta
    void Update();                                 // recompute position

    glm::mat4 GetViewMatrix() const { return glm::lookAt(m_Position, m_LookAt, glm::vec3(0,1,0)); }
    const glm::vec3& GetPosition() const { return m_Position; }
    const glm::vec3& GetForward()  const { return m_Forward; }
    const glm::vec3& GetRight()    const { return m_Right; }

private:
    void updateVectors();

    const MovableObject* m_Target; // followed target (not owned)

    // Orbit parameters
    float m_Distance;
    float m_MinDistance;
    float m_MaxDistance;
    float m_Pitch;      // degrees
    float m_MinPitch;
    float m_MaxPitch;
    float m_Yaw;        // degrees
    float m_TargetYOffset;

    // Derived state
    glm::vec3 m_Position;
    glm::vec3 m_LookAt;
    glm::vec3 m_Forward;
    glm::vec3 m_Right;
};
