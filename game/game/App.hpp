#pragma once

#include "Window.hpp"
#include "Input.hpp"

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
