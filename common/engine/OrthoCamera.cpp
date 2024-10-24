#include "engine/OrthoCamera.hpp"

#include <glm/glm.hpp>
#include <glm/ext/matrix_clip_space.hpp>
#include <glm/gtc/matrix_transform.hpp>

OrthoCamera::OrthoCamera(float left, float right, float bottom, float top)
    : m_near(-1.0f), m_far(1.0f), m_projectionMatrix(glm::ortho(left, right, bottom, top, m_near, m_far)), m_viewMatrix(1.0f)
{
    m_viewProjectionMatrix = m_projectionMatrix * m_viewMatrix;
}

void OrthoCamera::setProjection(float left, float right, float bottom, float top)
{
    m_projectionMatrix = glm::ortho(left, right, bottom, top, -1.0f, 1.0f);
    m_viewProjectionMatrix = m_projectionMatrix * m_viewMatrix;
}

void OrthoCamera::recalculateViewProjectionMatrix()
{
    glm::mat4 transform = glm::translate(glm::mat4(1.0f), m_position) *
        glm::rotate(glm::mat4(1.0f), glm::radians(m_rotation.z), glm::vec3(0, 0, 1));

    m_viewMatrix = glm::inverse(transform);
    m_viewProjectionMatrix = m_projectionMatrix * m_viewMatrix;
}

