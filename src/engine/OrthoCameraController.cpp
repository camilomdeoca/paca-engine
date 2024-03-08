#include "OrthoCameraController.hpp"
#include "Input.hpp"

OrthoCameraController::OrthoCameraController(float aspectRatio)
    : m_aspectRatio(aspectRatio), m_camera(-m_aspectRatio*m_zoom, m_aspectRatio*m_zoom, -m_zoom, m_zoom)
{
    std::function<void(MouseWheelEvent&)> callback = [this](MouseWheelEvent &event) { onMouseScroll(event); };
    Input::addMouseWheelDownCallback(callback); // TODO: remove callback on destructor
    Input::addMouseWheelUpCallback(callback); // TODO: remove callback on destructor
}

void OrthoCameraController::onUpdate(float ms)
{
    constexpr float cameraSpeed = 0.003f; // units per ms
    constexpr float rotationSpeed = 0.1f; // degrees per ms

    glm::vec3 position = getCamera().getPosition();
    float rotation = getCamera().getRotation();
    
    if (Input::isKeyPressed(Key::a)) {
        position.x -= cos(glm::radians(rotation)) * cameraSpeed * ms * m_zoom;
        position.y -= sin(glm::radians(rotation)) * cameraSpeed * ms * m_zoom;
    }
    if (Input::isKeyPressed(Key::d)) {
        position.x += cos(glm::radians(rotation)) * cameraSpeed * ms * m_zoom;
        position.y += sin(glm::radians(rotation)) * cameraSpeed * ms * m_zoom;
    }
    if (Input::isKeyPressed(Key::w)) {
        position.x += -sin(glm::radians(rotation)) * cameraSpeed * ms * m_zoom;
        position.y += cos(glm::radians(rotation)) * cameraSpeed * ms * m_zoom;
    }
    if (Input::isKeyPressed(Key::s)) {
        position.x -= -sin(glm::radians(rotation)) * cameraSpeed * ms * m_zoom;
        position.y -= cos(glm::radians(rotation)) * cameraSpeed * ms * m_zoom;
    }

    if (Input::isKeyPressed(Key::q)) {
        rotation -= rotationSpeed * ms;
    }
    if (Input::isKeyPressed(Key::e)) {
        rotation += rotationSpeed * ms;
    }
    if (rotation > 180.0f) rotation -= 360.0f;
    if (rotation <= -180.0f) rotation += 360.0f;

    getCamera().setRotation(rotation);
    getCamera().setPosition(position);
}

void OrthoCameraController::onMouseScroll(MouseWheelEvent &event)
{
    m_zoom *= 1.0f - event.amount * 0.1f;
    // m_zoom -= event.amount * 0.25f;
    m_camera.setProjection(-m_aspectRatio*m_zoom, m_aspectRatio*m_zoom, -m_zoom, m_zoom);
}


