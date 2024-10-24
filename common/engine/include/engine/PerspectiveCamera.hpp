#pragma once

#include "engine/Camera.hpp"

class PerspectiveCamera : public Camera {
public:
    PerspectiveCamera(float aspect, float fov);
    ~PerspectiveCamera();

    const glm::vec3 &getPosition() const override { return m_position; }
    void setPosition(const glm::vec3 &position);
    // Pitch, yaw, roll
    const glm::vec3 &getRotation() const override { return m_rotation; }
    void setRotation(const glm::vec3 &rotation);
    const glm::vec3 &getDirection() const override { return m_direction; }
    const glm::vec3 &getUp() const override { return m_up; };
    const glm::mat4 &getViewProjectionMatrix() const override;
    const glm::mat4 &getProjectionMatrix() const override;
    const glm::mat4 &getViewMatrix() const override;

    float getFov() const { return m_fov; }
    float getAspect() const { return m_aspect; }
    float getNear() const override { return m_near; }
    float getFar() const override { return m_far; }
    void setFov(float fov);
    void setAspect(float aspect);

private:
    mutable bool m_projectionMatrixNeedsRecalculation = true;
    mutable bool m_viewMatrixNeedsRecalculation = true;
    mutable bool m_viewProjectionMatrixNeedsRecalculation = true;
    mutable bool m_directionNeedsRecalculation = true;
    float m_aspect, m_fov, m_near, m_far;
    mutable glm::mat4 m_projectionMatrix;
    mutable glm::mat4 m_viewMatrix;
    mutable glm::mat4 m_viewProjectionMatrix;

    mutable glm::vec3 m_direction;
    glm::vec3 m_position = { 0.0f, 0.0f, 2.0f };
    glm::vec3 m_rotation = { 0.0f, 0.0f, 0.0f };
    glm::vec3 m_up = { 0.0f, 1.0f, 0.0f}; // No roll
};
