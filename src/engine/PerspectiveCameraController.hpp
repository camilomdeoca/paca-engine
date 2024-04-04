#pragma once

#include "engine/Input.hpp"
#include "engine/PerspectiveCamera.hpp"

class PerspectiveCameraController {
public:
    PerspectiveCameraController(float aspect, float fov);
    ~PerspectiveCameraController();

    void onUpdate(float ms);

    const PerspectiveCamera &getCamera() { return m_camera; }
    void setAspect(float aspect) { m_camera.setAspect(aspect); }

    void onMouseScroll(const MouseWheelEvent &event);
    void onMouseMotion(const MouseMotionEvent &event);

    //void pauseControl();
    //void resumeControl();

private:
    PerspectiveCamera m_camera;
    EventReceiver m_eventReceiver;
    bool m_haveControl = true;
};

