#pragma once

#include "engine/ForwardRenderer.hpp"
#include "game/Window.hpp"
#include "game/Input.hpp"

class App {
public:
    App();
    ~App();

    void init(std::string title = "Engine");
    void run();

private:
    Window m_window;
    ForwardRenderer m_renderer;
    EventReceiver m_eventReceiver;
    bool m_running = true;
};
