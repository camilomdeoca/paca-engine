#include "App.hpp"

#include "ui/UI.hpp"

#include <engine/FlecsSerialization.hpp>
#include <engine/Loader.hpp>

#include <engine/SceneManager.hpp>
#include <engine/Input.hpp>
#include <engine/Action.hpp>
#include <engine/BinarySerialization.hpp>
#include <engine/YamlSerialization.hpp>
#include <utils/Assert.hpp>
#include <engine/OrthoCamera.hpp>
#include <engine/AssetManager.hpp>
#include <opengl/gl.hpp>

#include <SDL2/SDL.h>
#include <format>
#include <glm/glm.hpp>
#include <string>

#include <imgui.h>
#include <ImGuizmo.h>
#include <implot.h>
#include <backends/imgui_impl_sdl2.h>
#include <backends/imgui_impl_opengl3.h>
#include <yaml-cpp/node/parse.h>

App::App()
    : m_assetMetadataManager(m_assetManager)
{}

App::~App()
{
    BindingsManager::shutdown();
    SDL_Quit();

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImPlot::DestroyContext();
    ImGui::DestroyContext();
}

void App::init(std::string title)
{
    if (SDL_Init(SDL_INIT_EVERYTHING) < 0) {
        ERROR("Error initializing SDL: {}", SDL_GetError());
        ASSERT(false);
    }

    m_window.create(title);

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

    engine::ForwardRenderer::Flags flags =
        //engine::ForwardRenderer::Flags::enableParallaxMapping |
        engine::ForwardRenderer::Flags::enableShadowMapping;

    GL::init();
    m_assetMetadataManager.init();
    Input::init();
    BindingsManager::init();
    m_renderer.init(flags);
    //Renderer2D::init();
    
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImPlot::CreateContext();
    ImGuiIO &io = ImGui::GetIO();
    (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

    ImGui::StyleColorsDark();

    ImGui_ImplSDL2_InitForOpenGL(m_window.getSDLWindow(), m_window.getSDLGLContext());
    ImGui_ImplOpenGL3_Init("#version 450 core");

    m_eventReceiver.setEventHandler([this] (const Event &event) {
        switch (event.getType()) {
        case EventType::exit:
            m_running = false;
            break;
        default: break;
        }
    });
    m_eventReceiver.setEventsMask(EventMask::exit);
}

void App::run()
{
    //Input::restrainMouseToWindow(true);
    //PerspectiveCamera camera((float)m_window.getWidth() / m_window.getHeight(), 90.0f);
    //OrthoCamera uiCamera(0.0f, m_window.getWidth(), 0.0f, m_window.getHeight());

    //m_resourceManager.loadAssetPack("build/out.pack");
    flecs::world world;
    {
        engine::serializers::FlecsUnserializer unserializer("flecs.yaml");
        unserializer(world);
    }

    paca::fileformats::NewAssetPack assetPack;
    {
        engine::serializers::YamlUnserializer unserializer("assets.yaml");
        unserializer(assetPack);
    }

    for (auto &staticMeshWithPath : assetPack.staticMeshes)
    {
        m_assetMetadataManager.add(staticMeshWithPath);
    }
    for (auto &animatedMeshWithPath : assetPack.animatedMeshes)
    {
        m_assetMetadataManager.add(animatedMeshWithPath);
    }
    for (auto &animationWithPath : assetPack.animations)
    {
        m_assetMetadataManager.add(animationWithPath);
    }
    for (auto &textureWithPath : assetPack.textures)
    {
        auto texture = engine::loaders::load<paca::fileformats::Texture>(textureWithPath.path.c_str());
        if (!texture)
        {
            ERROR("Couldn't load texture: {}", textureWithPath.path);
            continue;
        }

        texture->name = textureWithPath.name;
        texture->id = textureWithPath.id;
        m_assetManager.add(*texture);
    }
    for (auto &cubemapWithPath : assetPack.cubeMaps)
    {
        auto cubemap = engine::loaders::load<paca::fileformats::CubeMap>(cubemapWithPath.path.c_str());
        if (!cubemap)
        {
            ERROR("Couldn't load cubemap: {}", cubemapWithPath.path);
            continue;
        }
        cubemap->name = cubemapWithPath.name;
        cubemap->id = cubemapWithPath.id;
        m_assetManager.add(*cubemap);
    }
    INFO("MATERIALS: {}", assetPack.materials.size());
    for (auto &material : assetPack.materials)
    {
        m_assetMetadataManager.add(material);
    }

    BindingsManager::bind(Key::w, "forward");
    BindingsManager::bind(Key::s, "backward");
    BindingsManager::bind(Key::a, "left");
    BindingsManager::bind(Key::d, "right");
    BindingsManager::bind(Key::space, "up");
    BindingsManager::bind(Key::lshift, "down");
    BindingsManager::bindMouseWheelUp("zoom_in");
    BindingsManager::bindMouseWheelDown("zoom_out");

    Action wireframeModeToggle;
    wireframeModeToggle.init("wireframe_toggle", []() {
        static bool wireframeEnabled = false;
        wireframeEnabled = !wireframeEnabled;
        GL::setPolygonMode(wireframeEnabled ? GL::PolygonMode::wireframe : GL::PolygonMode::solid);
    });
    BindingsManager::bind(Key::q, "wireframe_toggle");

    float lastFrameTime = SDL_GetTicks();
    UI ui({
        .assetManager = m_assetManager,
        .assetMetadataManager = m_assetMetadataManager,
        .renderer = m_renderer,
        .world = world,
    });

    while (m_running) {
        float time = SDL_GetTicks();
        float timeDelta = time - lastFrameTime;
        lastFrameTime = time;

        ui.update(timeDelta);

        SDL_Event event;
        while (SDL_PollEvent(&event))
        {
            // If event is for imgui dont process it
            if (ImGui_ImplSDL2_ProcessEvent(&event))
                continue;

            Input::processSDLEvent(event);
        }

        /* Draw */
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplSDL2_NewFrame();
        ImGui::NewFrame();
        ImGuizmo::BeginFrame();

        ui.draw();

        FrameBuffer::getDefault().bind();
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        m_window.swapBuffers();
    }
}
