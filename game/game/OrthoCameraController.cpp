#include "game/OrthoCameraController.hpp"
#include "game/Input.hpp"

OrthoCameraController::OrthoCameraController(float aspectRatio)
    : m_aspectRatio(aspectRatio), m_camera(-m_aspectRatio*m_zoom, m_aspectRatio*m_zoom, -m_zoom, m_zoom)
{
    m_eventReceiver.setEventsMask(EventMask::mouseWheelDown | EventMask::mouseWheelUp);
    m_eventReceiver.setEventHandler([this] (const Event &event) {
        switch (event.getType()) {
        case EventType::mouseWheelDown:
        case EventType::mouseWheelUp:
            onMouseScroll(static_cast<const MouseWheelEvent&>(event));
        default: break;
        }
    });
}

OrthoCameraController::~OrthoCameraController()
{}

void OrthoCameraController::onUpdate(float ms)
{
    constexpr float cameraSpeed = 0.003f; // units per ms
    constexpr float rotationSpeed = 0.1f; // degrees per ms

    glm::vec3 position = getCamera().getPosition();
    glm::vec3 rotation = getCamera().getRotation();
    
    if (Input::isKeyPressed(Key::a)) {
        position.x -= cos(glm::radians(rotation.z)) * cameraSpeed * ms * m_zoom;
        position.y -= sin(glm::radians(rotation.z)) * cameraSpeed * ms * m_zoom;
    }
    if (Input::isKeyPressed(Key::d)) {
        position.x += cos(glm::radians(rotation.z)) * cameraSpeed * ms * m_zoom;
        position.y += sin(glm::radians(rotation.z)) * cameraSpeed * ms * m_zoom;
    }
    if (Input::isKeyPressed(Key::w)) {
        position.x += -sin(glm::radians(rotation.z)) * cameraSpeed * ms * m_zoom;
        position.y += cos(glm::radians(rotation.z)) * cameraSpeed * ms * m_zoom;
    }
    if (Input::isKeyPressed(Key::s)) {
        position.x -= -sin(glm::radians(rotation.z)) * cameraSpeed * ms * m_zoom;
        position.y -= cos(glm::radians(rotation.z)) * cameraSpeed * ms * m_zoom;
    }

    if (Input::isKeyPressed(Key::q)) {
        rotation -= rotationSpeed * ms;
    }
    if (Input::isKeyPressed(Key::e)) {
        rotation += rotationSpeed * ms;
    }
    if (rotation.z > 180.0f) rotation.z -= 360.0f;
    if (rotation.z <= -180.0f) rotation.z += 360.0f;

    getCamera().setRotation(rotation);
    getCamera().setPosition(position);
}

void OrthoCameraController::onMouseScroll(const MouseWheelEvent &event)
{
    m_zoom *= 1.0f - event.getAmmount() * 0.1f;
    // m_zoom -= event.getAmount() * 0.25f;
    m_camera.setProjection(-m_aspectRatio*m_zoom, m_aspectRatio*m_zoom, -m_zoom, m_zoom);
}


