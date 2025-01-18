#pragma once

#include <engine/ForwardRenderer.hpp>
#include <engine/Window.hpp>
#include <engine/Input.hpp>

class App {
public:
    App();
    ~App();

    void init(std::string title = "Engine");
    void run();

private:
    Window m_window;
    engine::ForwardRenderer m_renderer;
    AssetManager m_assetManager;
    EventReceiver m_eventReceiver;
    bool m_running = true;
};
