#pragma once

#include <engine/ForwardRenderer.hpp>
#include <engine/Window.hpp>
#include <engine/Input.hpp>

class App {
public:
    App();
    ~App();

    void init(std::string title = "Editor");
    void run();

private:
    Window m_window;
    FrameBuffer m_sceneRendererTarget;
    engine::ForwardRenderer m_renderer;
    NewResourceManager m_resourceManager;
    EventReceiver m_eventReceiver;
    bool m_running = true;
};
