#include "App.hpp"
#include "UI.hpp"
#include <engine/SceneManager.hpp>
#include <engine/Input.hpp>
#include <engine/Action.hpp>
#include <serializers/BinarySerialization.hpp>
#include <utils/Assert.hpp>
#include <engine/OrthoCamera.hpp>
#include <engine/NewResourceManager.hpp>
#include <opengl/gl.hpp>

#include <SDL2/SDL.h>
#include <format>
#include <glm/glm.hpp>
#include <string>

#include <imgui.h>
#include <ImGuizmo.h>
#include <backends/imgui_impl_sdl2.h>
#include <backends/imgui_impl_opengl3.h>

App::App()
{}

App::~App()
{
    BindingsManager::shutdown();
    SDL_Quit();
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

    m_sceneRendererTarget.init({
        .width = 320,
        .height = 240,
        .textureAttachmentFormats = {
            Texture::Format::depth24,
            Texture::Format::RGBA8,
        },
    });

    engine::ForwardRenderer::Parameters rendererParams;
    rendererParams.width = 320;
    rendererParams.height = 240;
    rendererParams.flags =
        //ForwardRenderer::Parameters::enableParallaxMapping |
        engine::ForwardRenderer::Parameters::enableShadowMapping;

    GL::init();
    Input::init();
    BindingsManager::init();
    m_renderer.init(rendererParams);
    m_renderer.setRenderTarget(m_sceneRendererTarget);
    //Renderer2D::init();
    
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
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
    PerspectiveCamera camera((float)m_window.getWidth() / m_window.getHeight(), 90.0f);
    OrthoCamera uiCamera(0.0f, m_window.getWidth(), 0.0f, m_window.getHeight());

    m_resourceManager.loadAssetPack("build/out.pack");
    
    engine::SceneManager sceneManager;
    paca::fileformats::Scene scene;
    {
        serialization::BinaryUnserializer unserializer("build/scene.scene");
        unserializer(scene);
    }
    sceneManager.loadScene(scene);
    flecs::world &world = sceneManager.getFlecsWorld();

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

    ui::EditorContext editorContext = {
        .camera = camera,
        .resourceManager = m_resourceManager,
        .renderTarget = m_sceneRendererTarget,
        .resizeCallback = [this, &camera](uint32_t w, uint32_t h) {
            m_sceneRendererTarget.shutdown();
            m_sceneRendererTarget.init({
                .width = w,
                .height = h,
                .textureAttachmentFormats = {
                    Texture::Format::depth24,
                    Texture::Format::RGBA8,
                },
            });
            m_renderer.resize(w, h);
            camera.setAspect((float)w/h);
        },
        .world = sceneManager.getFlecsWorld(),
        .selectedEntity = sceneManager.getFlecsWorld().lookup("1002"),
    };

    float lastFrameTime = SDL_GetTicks();
    while (m_running) {
        float time = SDL_GetTicks();
        float timeDelta = time - lastFrameTime;
        lastFrameTime = time;

        constexpr int frameTimeMeasurementCount = 10;
        static float framTimeBuffer[frameTimeMeasurementCount] = {0};
        static int oldestFrameTimeIndex = 0;
        framTimeBuffer[oldestFrameTimeIndex] = timeDelta;
        oldestFrameTimeIndex++;
        if (oldestFrameTimeIndex >= frameTimeMeasurementCount) oldestFrameTimeIndex = 0;
        float smoothFrameTime = 0;
        for (int i = 0; i < frameTimeMeasurementCount; i++) smoothFrameTime += framTimeBuffer[i]/frameTimeMeasurementCount;

        SDL_Event event;
        while (SDL_PollEvent(&event))
        {
            // If event is for imgui dont process it
            if (ImGui_ImplSDL2_ProcessEvent(&event))
                continue;

            Input::processSDLEvent(event);
        }
        
        // Rotate Light
        //static float distance = 3.0f;
        //glm::vec3 newLightPos(distance * sin(time*0.002f), 2.0f, distance * cos(time*0.002f));
        //light->setPosition(newLightPos);
        //lightBulb.setPosition(newLightPos);        

        /* Draw */
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplSDL2_NewFrame();
        ImGui::NewFrame();
        ImGuizmo::BeginFrame();

        ui::draw(editorContext);

        m_sceneRendererTarget.bind();
        GL::clear();
        m_renderer.renderWorld(camera, world, m_resourceManager);
        m_sceneRendererTarget.unbind();

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        m_window.swapBuffers();
    }
}
