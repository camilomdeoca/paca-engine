#pragma once

#include <engine/ForwardRenderer.hpp>
#include "AssetMetadataManager.hpp"
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
    engine::ForwardRenderer m_renderer;
    AssetManager m_assetManager;
    AssetMetadataManager m_assetMetadataManager;
    EventReceiver m_eventReceiver;
    bool m_running = true;
};
