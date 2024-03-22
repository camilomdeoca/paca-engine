#pragma once

#include "Input.hpp"
#include "OrthoCamera.hpp"

class OrthoCameraController {
public:
    OrthoCameraController(float aspectRatio);
    ~OrthoCameraController();

    void onUpdate(float ms);

    //void onKeyPress(KeyPressEvent &event);
    OrthoCamera &getCamera() { return m_camera; }

    void onMouseScroll(MouseWheelEvent &event);

private:
    float m_aspectRatio;
    float m_zoom = 1.0f;
    OrthoCamera m_camera;

    std::list<MouseWheelCallback>::const_iterator m_mouseWheelUpCallbackReference, m_mouseWheelDownCallbackReference;
};
