#include "PerspectiveCamera.hpp"

#include <glm/gtc/matrix_transform.hpp>

PerspectiveCamera::PerspectiveCamera(float aspect, float fov)
    : m_aspect(aspect), m_fov(fov), m_near(0.1f), m_far(100.0f)
{
}

PerspectiveCamera::~PerspectiveCamera()
{}

void PerspectiveCamera::setRotation(const glm::vec3 &rotation)
{
    m_rotation = rotation;
    m_directionNeedsRecalculation = true;
    m_viewMatrixNeedsRecalculation = true;
    m_viewProjectionMatrixNeedsRecalculation = true;
}

const glm::mat4 &PerspectiveCamera::getViewProjectionMatrix() const
{
    if (m_viewProjectionMatrixNeedsRecalculation)
    {
        m_viewProjectionMatrix = getProjectionMatrix() * getViewMatrix();
        m_viewProjectionMatrixNeedsRecalculation = false;
    }
    return m_viewProjectionMatrix;
}

const glm::mat4 &PerspectiveCamera::getProjectionMatrix() const
{
    if (m_projectionMatrixNeedsRecalculation)
    {
        m_projectionMatrix = glm::perspective(glm::radians(m_fov), m_aspect, m_near, m_far);
        m_projectionMatrixNeedsRecalculation = false;
    }
    return m_projectionMatrix;
}

const glm::mat4 &PerspectiveCamera::getViewMatrix() const
{
    if (m_viewMatrixNeedsRecalculation)
    {
        if (m_directionNeedsRecalculation)
        {
            m_direction = glm::normalize(glm::vec3(
                cos(glm::radians(m_rotation.y)) * cos(glm::radians(m_rotation.x)),
                sin(glm::radians(m_rotation.x)),
                sin(glm::radians(m_rotation.y)) * cos(glm::radians(m_rotation.x))
            ));
            m_directionNeedsRecalculation = false;
        }
        m_viewMatrix = glm::lookAt(m_position, m_position + m_direction, m_up);
        m_viewMatrixNeedsRecalculation = false;
    }
    return m_viewMatrix;
};

void PerspectiveCamera::setPosition(const glm::vec3 &position)
{
    m_position = position;
    m_viewMatrixNeedsRecalculation = true;
    m_viewProjectionMatrixNeedsRecalculation = true;
}

void PerspectiveCamera::setFov(float fov)
{
    m_fov = fov;
    m_projectionMatrixNeedsRecalculation = true;
    m_viewProjectionMatrixNeedsRecalculation = true;
}

void PerspectiveCamera::setAspect(float aspect)
{
    m_aspect = aspect;
    m_projectionMatrixNeedsRecalculation = true;
    m_viewProjectionMatrixNeedsRecalculation = true;
}
