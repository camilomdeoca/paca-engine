#include "PerspectiveCameraController.hpp"

#include "engine/Input.hpp"

#include <algorithm>

PerspectiveCameraController::PerspectiveCameraController(float aspect, float fov)
    : m_camera(aspect, fov)
{
    m_eventReceiver.setEventsMask(EventMask::mouseWheelDown
            | EventMask::mouseWheelUp
            | EventMask::mouseMotion);
    m_eventReceiver.setEventHandler([this] (const Event &event) {
        switch (event.getType()) {
        case EventType::mouseMotion:
            onMouseMotion(static_cast<const MouseMotionEvent&>(event));
            break;
        case EventType::mouseWheelDown:
        case EventType::mouseWheelUp:
            onMouseScroll(static_cast<const MouseWheelEvent&>(event));
            break;
        default: break;
        }
    });
}

PerspectiveCameraController::~PerspectiveCameraController()
{}

void PerspectiveCameraController::onUpdate(float ms)
{
    if (m_haveControl) {
        constexpr float cameraSpeed = 0.005f; // units per ms

        glm::vec3 position = getCamera().getPosition();
        glm::vec3 front = getCamera().getDirection();
        front.y = 0;
        front = glm::normalize(front);
        glm::vec3 right = glm::normalize(glm::cross(front, getCamera().getUp()));

        if (Input::isKeyPressed(Key::a)) {
            position -= right * cameraSpeed * ms;
        }
        if (Input::isKeyPressed(Key::d)) {
            position += right * cameraSpeed * ms;
        }
        if (Input::isKeyPressed(Key::w)) {
            position += front * cameraSpeed * ms;
        }
        if (Input::isKeyPressed(Key::s)) {
            position -= front * cameraSpeed * ms;
        }
        if (Input::isKeyPressed(Key::space)) {
            position.y += cameraSpeed * ms;
        }
        if (Input::isKeyPressed(Key::lshift)) {
            position.y -= cameraSpeed * ms;
        }

        m_camera.setPosition(position);
    }
}

void PerspectiveCameraController::onMouseScroll(const MouseWheelEvent &event)
{
    float fov = m_camera.getFov();
    fov *= 1.0f - event.getAmmount() * 0.1f;
    if (fov > 120.0f) fov = 120.0f;
    // fov -= event.amount * 0.25f;
    m_camera.setFov(fov);
}

void PerspectiveCameraController::onMouseMotion(const MouseMotionEvent &event)
{
    const float sensitivity = 0.1f;
    glm::vec3 rotation = m_camera.getRotation();
    float fov = m_camera.getFov();

    rotation.x -= event.getRelativeY() * sensitivity * (fov/90.0f);
    rotation.y += event.getRelativeX() * sensitivity * (fov/90.0f);

    rotation.x = std::clamp(rotation.x, -89.0f, 89.0f);

    m_camera.setRotation(rotation);
}
