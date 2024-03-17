#pragma once

#include "engine/Camera.hpp"

#include <glm/glm.hpp>

class OrthoCamera : public Camera {
public:
    OrthoCamera(float left, float right, float bottom, float top);

    void setProjection(float left, float right, float bottom, float top);

    const glm::vec3 &getPosition() const override { return m_position; }
    void setPosition(const glm::vec3 &position) { m_position = position; recalculateViewProjectionMatrix(); }

    const glm::vec3 &getRotation() const override { return m_rotation; }
    void setRotation(const glm::vec3 &rotation) { m_rotation = rotation; recalculateViewProjectionMatrix(); }

    const glm::vec3 &getDirection() const override { return m_direction; }
    const glm::vec3 &getUp() const override { return m_up; }

    const glm::mat4 &getProjectionMatrix() const override { return m_projectionMatrix; }
    const glm::mat4 &getViewMatrix() const override { return m_viewMatrix; }
    const glm::mat4 &getViewProjectionMatrix() const override { return m_viewProjectionMatrix; }

private:
    void recalculateViewProjectionMatrix();

    glm::mat4 m_projectionMatrix;
    glm::mat4 m_viewMatrix;
    glm::mat4 m_viewProjectionMatrix;

    glm::vec3 m_position = { 0.0f, 0.0f, 0.0f };
    glm::vec3 m_rotation;
    glm::vec3 m_direction;
    glm::vec3 m_up;
};
