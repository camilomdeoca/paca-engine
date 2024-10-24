#pragma once

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
    EventReceiver m_eventReceiver;
    bool m_running = true;
};
