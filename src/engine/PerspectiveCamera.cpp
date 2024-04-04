#include "PerspectiveCamera.hpp"

#include <glm/gtc/matrix_transform.hpp>

PerspectiveCamera::PerspectiveCamera(float aspect, float fov)
    : m_aspect(aspect), m_fov(fov), m_projectionMatrix(glm::perspective(glm::radians(m_fov), m_aspect, 0.1f, 10000.0f))
{
    m_direction = glm::normalize(glm::vec3(
        cos(glm::radians(m_rotation.y)) * cos(glm::radians(m_rotation.x)),
        sin(glm::radians(m_rotation.x)),
        sin(glm::radians(m_rotation.y)) * cos(glm::radians(m_rotation.x))
    ));
}

PerspectiveCamera::~PerspectiveCamera()
{}

void PerspectiveCamera::setRotation(const glm::vec3 &rotation)
{
    m_rotation = rotation;
    m_direction = glm::normalize(glm::vec3(
        cos(glm::radians(m_rotation.y)) * cos(glm::radians(m_rotation.x)),
        sin(glm::radians(m_rotation.x)),
        sin(glm::radians(m_rotation.y)) * cos(glm::radians(m_rotation.x))
    ));
    recalculateViewProjectionMatrix();
}

void PerspectiveCamera::setFov(float fov)
{
    m_fov = fov;
    m_projectionMatrix = glm::perspective(glm::radians(m_fov), m_aspect, 0.1f, 10000.0f);
    recalculateViewProjectionMatrix();
}

void PerspectiveCamera::setAspect(float aspect)
{
    m_aspect = aspect;
    m_projectionMatrix = glm::perspective(glm::radians(m_fov), m_aspect, 0.1f, 10000.0f);
    recalculateViewProjectionMatrix();
}

void PerspectiveCamera::recalculateViewProjectionMatrix()
{
    glm::mat4 transform = glm::lookAt(m_position, m_position + m_direction, m_up);
    m_viewMatrix = transform;
    m_viewProjectionMatrix = m_projectionMatrix * m_viewMatrix;
}
