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
    engine::ForwardRenderer m_renderer;
    NewResourceManager m_resourceManager;
    EventReceiver m_eventReceiver;
    bool m_running = true;
};
