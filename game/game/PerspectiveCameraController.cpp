#include "game/PerspectiveCameraController.hpp"

#include "utils/Assert.hpp"
#include "game/Input.hpp"

#include <algorithm>

PerspectiveCameraController::PerspectiveCameraController(float aspect, float fov)
    : m_camera(aspect, fov)
{
    m_actions[0].init("forward",
            [this]() { m_moving |= DirectionMask::forward; },
            [this]() { m_moving &= ~DirectionMask::forward; });
    m_actions[1].init("backward",
            [this]() { m_moving |= DirectionMask::backward; },
            [this]() { m_moving &= ~DirectionMask::backward; });
    m_actions[2].init("left",
            [this]() { m_moving |= DirectionMask::left; },
            [this]() { m_moving &= ~DirectionMask::left; });
    m_actions[3].init("right",
            [this]() { m_moving |= DirectionMask::right; },
            [this]() { m_moving &= ~DirectionMask::right; });
    m_actions[4].init("up",
            [this]() { m_moving |= DirectionMask::up; },
            [this]() { m_moving &= ~DirectionMask::up; });
    m_actions[5].init("down",
            [this]() { m_moving |= DirectionMask::down; },
            [this]() { m_moving &= ~DirectionMask::down; });
    m_actions[6].init("zoom_in", [this]() { m_camera.setFov(std::clamp(m_camera.getFov() * 1/1.1f, 0.0f, 120.0f)); });
    m_actions[7].init("zoom_out", [this]() { m_camera.setFov(std::clamp(m_camera.getFov() * 1.1f, 0.0f, 120.0f)); });

    m_eventReceiver.setEventHandler([this](const Event &event) {
        switch (event.getType()) {
        case EventType::mouseMotion:
            onMouseMotion(static_cast<const MouseMotionEvent&>(event));
            break;
        default:
            ASSERT_MSG(false, "Received unrequested event type");
            break;
        }
    });
    m_eventReceiver.setEventsMask(EventMask::mouseMotion);
}

PerspectiveCameraController::~PerspectiveCameraController()
{}

void PerspectiveCameraController::onUpdate(float ms)
{
    if (!m_haveControl) return;

    constexpr float cameraSpeed = 0.005f; // units per ms

    glm::vec3 position = getCamera().getPosition();
    glm::vec3 front = getCamera().getDirection();
    front.y = 0;
    front = glm::normalize(front);
    glm::vec3 right = glm::normalize(glm::cross(front, getCamera().getUp()));

    if (m_moving & DirectionMask::left) {
        position -= right * cameraSpeed * ms;
    }
    if (m_moving & DirectionMask::right) {
        position += right * cameraSpeed * ms;
    }
    if (m_moving & DirectionMask::forward) {
        position += front * cameraSpeed * ms;
    }
    if (m_moving & DirectionMask::backward) {
        position -= front * cameraSpeed * ms;
    }
    if (m_moving & DirectionMask::up) {
        position.y += cameraSpeed * ms;
    }
    if (m_moving & DirectionMask::down) {
        position.y -= cameraSpeed * ms;
    }

    m_camera.setPosition(position);
}

void PerspectiveCameraController::onMouseMotion(const MouseMotionEvent &event)
{
    if (!m_haveControl) return;

    const float sensitivity = 0.1f;
    glm::vec3 rotation = m_camera.getRotation();
    float fov = m_camera.getFov();

    rotation.x -= event.getRelativeY() * sensitivity * (fov/90.0f);
    rotation.y += event.getRelativeX() * sensitivity * (fov/90.0f);

    rotation.x = std::clamp(rotation.x, -89.0f, 89.0f);

    m_camera.setRotation(rotation);
}
