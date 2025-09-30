#include "../../include/camera/Camera.h"
#include "../../include/objects/MovableObject.h"
#include <algorithm>
#include <cmath>

Camera::Camera(const MovableObject* target)
    : m_Target(target),
      m_Distance(6.0f), m_MinDistance(2.5f), m_MaxDistance(25.0f),
      m_Pitch(25.0f), m_MinPitch(5.0f), m_MaxPitch(80.0f),
      m_Yaw(0.0f), m_TargetYOffset(1.0f),
      m_Position(0), m_LookAt(0), m_Forward(0,0,-1), m_Right(1,0,0) {
    Update();
}

void Camera::ProcessMouseMovement(float dx, float dy) {
    const float rotateSpeed = 0.25f; // degrees per pixel
    m_Yaw   += dx * rotateSpeed;
    m_Pitch -= dy * rotateSpeed; // invert Y for standard feel
    if (m_Pitch < m_MinPitch) m_Pitch = m_MinPitch;
    if (m_Pitch > m_MaxPitch) m_Pitch = m_MaxPitch;
    Update();
}

void Camera::ProcessScroll(float dy) {
    m_Distance -= dy * 0.75f;
    m_Distance = std::clamp(m_Distance, m_MinDistance, m_MaxDistance);
    Update();
}

void Camera::Update() {
    if (!m_Target) return;
    glm::vec3 targetPos = m_Target->GetPosition();
    targetPos.y += m_TargetYOffset;

    float yawRad   = glm::radians(m_Yaw);
    float pitchRad = glm::radians(m_Pitch);

    float x = m_Distance * std::cos(pitchRad) * std::sin(yawRad);
    float y = m_Distance * std::sin(pitchRad);
    float z = m_Distance * std::cos(pitchRad) * std::cos(yawRad);

    m_Position = targetPos + glm::vec3(x, y, z);
    m_LookAt   = targetPos;
    updateVectors();
}

void Camera::updateVectors() {
    m_Forward = glm::normalize(m_LookAt - m_Position);
    m_Right   = glm::normalize(glm::cross(m_Forward, glm::vec3(0,1,0)));
}