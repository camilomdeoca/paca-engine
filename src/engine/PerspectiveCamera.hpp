#pragma once

#include "Camera.hpp"

class PerspectiveCamera : public Camera {
public:
    PerspectiveCamera(float aspect, float fov);
    ~PerspectiveCamera();

    const glm::vec3 &getPosition() const override { return m_position; }
    void setPosition(const glm::vec3 &position) { m_position = position; recalculateViewProjectionMatrix(); }
    // Pitch, yaw, roll
    const glm::vec3 &getRotation() const override { return m_rotation; }
    void setRotation(const glm::vec3 &rotation);
    const glm::vec3 &getDirection() const override { return m_direction; }
    const glm::vec3 &getUp() const override { return m_up; };
    const glm::mat4 &getViewProjectionMatrix() const override { return m_viewProjectionMatrix; }

    float getFov() { return m_fov; }
    void setFov(float fov);

private:
    void recalculateViewProjectionMatrix();

    float m_aspect, m_fov;
    glm::mat4 m_projectionMatrix;
    glm::mat4 m_viewMatrix;
    glm::mat4 m_viewProjectionMatrix;

    glm::vec3 m_position = { 0.0f, 0.0f, 2.0f };
    glm::vec3 m_rotation = { 0.0f, 0.0f, 0.0f };
    glm::vec3 m_direction;
    glm::vec3 m_right;
    glm::vec3 m_up = { 0.0f, 1.0f, 0.0f}; // No roll
};
