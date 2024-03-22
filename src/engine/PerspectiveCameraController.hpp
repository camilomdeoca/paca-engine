#pragma once

#include "engine/Input.hpp"
#include "engine/PerspectiveCamera.hpp"

class PerspectiveCameraController {
public:
    PerspectiveCameraController(float aspect, float fov);
    ~PerspectiveCameraController();

    void onUpdate(float ms);

    const PerspectiveCamera &getCamera() { return m_camera; }

    void onMouseScroll(MouseWheelEvent &event);
    void onMouseMotion(MouseMotionEvent &event);

    void pauseControl();
    void resumeControl();

private:
    PerspectiveCamera m_camera;
    std::list<MouseMotionCallback>::const_iterator m_mouseMotionCallbackReference;
    std::list<MouseWheelCallback>::const_iterator m_mouseWheelUpCallbackReference, m_mouseWheelDownCallbackReference;
    bool m_haveControl = false;
};

