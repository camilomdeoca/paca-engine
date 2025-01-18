#include "game/App.hpp"
#include <engine/SceneManager.hpp>
#include <engine/Input.hpp>
#include <engine/Action.hpp>
#include <serializers/BinarySerialization.hpp>
#include <utils/Assert.hpp>
#include <engine/OrthoCamera.hpp>
#include <engine/NewResourceManager.hpp>
#include <opengl/gl.hpp>
#include "game/PerspectiveCameraController.hpp"

#include <SDL2/SDL.h>
#include <format>
#include <glm/glm.hpp>
#include <string>

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

    engine::ForwardRenderer::Parameters rendererParams;
    rendererParams.width = m_window.getWidth();
    rendererParams.height = m_window.getHeight();
    rendererParams.flags =
        //ForwardRenderer::Parameters::enableParallaxMapping |
        engine::ForwardRenderer::Parameters::enableShadowMapping;

    GL::init();
    Input::init();
    BindingsManager::init();
    m_renderer.init(rendererParams);
    //Renderer2D::init();

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
    Input::restrainMouseToWindow(true);
    PerspectiveCameraController cameraController((float)m_window.getWidth() / m_window.getHeight(), 90.0f);
    OrthoCamera uiCamera(0.0f, m_window.getWidth(), 0.0f, m_window.getHeight());

    m_assetManager.loadAssetPack("build/out.pack");
    
    engine::SceneManager sceneManager;
    serialization::BinaryUnserializer unserializer("build/scene.scene");
    paca::fileformats::Scene scene;
    unserializer << scene;
    sceneManager.loadScene(scene);
    flecs::world &world = sceneManager.getFlecsWorld();

    //Action setLightPosAction[2];
    //setLightPosAction[0].init("setLight1", [&cameraController, &light0]() {
    //    glm::vec3 newPos = cameraController.getCamera().getPosition();
    //    INFO("LIGHT1");
    //    light0.ensure<engine::components::Transform>().position = newPos;
    //});
    //setLightPosAction[1].init("setLight2", [&cameraController, &light1]() {
    //    glm::vec3 newPos = cameraController.getCamera().getPosition();
    //    INFO("LIGHT2");
    //    light1.ensure<engine::components::Transform>().position = newPos;
    //});
    //BindingsManager::bind(Button::left, "setLight1");
    //BindingsManager::bind(Button::right, "setLight2");

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

    Action pause;
    pause.init("toggle_pause", [&cameraController]() {
        static bool val = true;
        val = !val;
        Input::restrainMouseToWindow(val);
        cameraController.setControl(val);
    });
    BindingsManager::bind(Key::esc, "toggle_pause");

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

        Input::processInput();

        cameraController.onUpdate(timeDelta);
        
        // Rotate Light
        //static float distance = 3.0f;
        //glm::vec3 newLightPos(distance * sin(time*0.002f), 2.0f, distance * cos(time*0.002f));
        //light->setPosition(newLightPos);
        //lightBulb.setPosition(newLightPos);        

        /* Draw */
        GL::setClearColor({0.1f, 0.1f, 0.15f, 1.0f});
        GL::clear();

        //RenderEnvironment environment;
        ////environment.pointLights = &lights;
        //environment.directionalLight = directionalLight;
        //environment.skybox = skybox;

        //Renderer::beginScene(cameraController.getCamera(), environment);
        const PerspectiveCamera &camera = cameraController.getCamera();
        m_renderer.renderWorld(camera, world, m_assetManager);

        // Render UI
        //Renderer2D::beginScene(uiCamera);
        //Renderer2D::drawString({0.0f, m_window.getHeight() - font.getHeight(), 0.5f}, std::format("{:.1f}", 1000.0f/smoothFrameTime) + "fps", font, {0.0f, 1.0f, 0.0f, 1.0f});
        //Renderer2D::drawString(
        //    {0.0f, m_window.getHeight() - font.getHeight() * 2.0f, 0.5f},
        //    "XYZ: " + std::format("{:.2f}", cameraController.getCamera().getPosition().x)
        //            + ", " + std::format("{:.2f}", cameraController.getCamera().getPosition().y)
        //            + ", " + std::format("{:.2f}", cameraController.getCamera().getPosition().z),
        //    font,
        //    {1.0f, 1.0f, 1.0f, 1.0f}
        //);
#define ENABLE_SHOW_DEPTH_TEXTURES
#ifdef ENABLE_SHOW_DEPTH_TEXTURES
        //const std::shared_ptr<Texture> &shadowMap = m_renderer.m_shadowMapAtlasFramebuffer->getDepthAttachment();
        //float width = shadowMap->getWidth() / 4.0f;
        //float height = shadowMap->getHeight() / 4.0f;
        //Renderer2D::drawQuad({0.0f, 0.0f, 0.0f}, {width, height}, shadowMap);
#endif // DEBUG
        //Renderer2D::endScene();

        m_window.swapBuffers();
    }
}
