#include "game/App.hpp"
#include "game/Input.hpp"
#include "game/Action.hpp"
#include "utils/Assert.hpp"
#include "engine/Light.hpp"
#include "engine/OrthoCamera.hpp"
#include "engine/ResourceManager.hpp"
#include "opengl/gl.hpp"
#include "engine/Renderer2D.hpp"
#include "game/PerspectiveCameraController.hpp"

#include <SDL2/SDL.h>
#include <format>
#include <glm/glm.hpp>
#include <memory>
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

    ForwardRenderer::Parameters rendererParams;
    rendererParams.width = m_window.getWidth();
    rendererParams.height = m_window.getHeight();
    rendererParams.shadowMapSize = 1024;
    rendererParams.viewFrustumSplits = {5.0f, 10.0f, 25.0f, 50.0f, 100.0f};
    rendererParams.flags =
        //ForwardRenderer::Parameters::enableParallaxMapping |
        ForwardRenderer::Parameters::enableShadowMapping;

    GL::init();
    Input::init();
    BindingsManager::init();
    m_renderer.init(rendererParams);
    Renderer2D::init();

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

    //Input::addResizeCallback([&cameraController](ResizeEvent &event) {
    //    GL::viewport(event.w, event.h);
    //    Renderer::resize(event.w, event.h);
    //    cameraController.setAspect((float)event.w/(float)event.h);
    //});

    Font font("assets/fonts/DejaVuSansFontAtlas.png", "assets/fonts/DejaVuSansFontAtlas.fntat");

    ResourceManager::loadAssetPack("build/out.pack");

    std::shared_ptr<Texture> skybox = ResourceManager::getCubeMap("skybox");

    std::shared_ptr<Model> vampire = ResourceManager::getModel("dancing_vampire");
    vampire->setScale(glm::vec3(0.01f));
    vampire->setPosition({0.0f, -0.2f, 0.0f});
    std::shared_ptr<Model> mainModel = ResourceManager::getModel("statue");
    mainModel->setScale(glm::vec3(10.0f));
    std::shared_ptr<Model> plane = ResourceManager::getModel("plane");
    plane->setPosition({0.0f, -1.0f, 0.0f});
    //std::shared_ptr<Model> lightBulb = ResourceManager::addModel("assets/models/lightBulb.pmdl");
    //lightBulb->setPosition({3.0f, 4.0f, -3.0f});
    //lightBulb->setRotation({-90.0f, 0.0f, 0.0f});
    //lightBulb->setScale(glm::vec3(3.0f));

    std::vector<std::shared_ptr<PointLight>> lights;
    lights.push_back(std::make_shared<PointLight>(glm::vec3(3.0f, 4.0f, -3.0f), glm::vec3(1.0f, 0.0f, 0.0f), 0.4f, 0.025f, nullptr));
    lights.push_back(std::make_shared<PointLight>(glm::vec3(3.0f, 4.0f, 3.0f), glm::vec3(0.0f, 0.0f, 1.0f), 0.4f, 0.025f, nullptr));
    std::shared_ptr<DirectionalLight> directionalLight = std::make_shared<DirectionalLight>(glm::vec3(2.0f, -4.0f, 1.0f), glm::vec3(1.0f, 1.0f, 0.9f), 0.7f);

    World world;
    world.addModels({ vampire, mainModel, plane });
    world.addDirectionalLight(directionalLight);
    world.addPointLights(lights);
    world.setSkybox(skybox);

    //Input::addKeyPressCallback([light](KeyPressEvent &event) {
    //    light->setIntensity(light->getIntensity() - 0.1);
    //    printf("intensity = %f\n", light->getIntensity());
    //}, Key::down);
    //Input::addKeyPressCallback([light](KeyPressEvent &event) {
    //    light->setIntensity(light->getIntensity() + 0.1);
    //    printf("intensity = %f\n", light->getIntensity());
    //}, Key::up);
    //Input::addKeyPressCallback([light](KeyPressEvent &event) {
    //    light->setAttenuation(light->getAttenuation() * 1.05);
    //    printf("attenuation = %f\n", light->getAttenuation());
    //}, Key::left);
    //Input::addKeyPressCallback([light](KeyPressEvent &event) {
    //    light->setAttenuation(light->getAttenuation() * 0.95);
    //    printf("attenuation = %f\n", light->getAttenuation());
    //}, Key::right);
    Action setLightPosAction[2];
    setLightPosAction[0].init("setLight1", [&lights, &cameraController]() {
        glm::vec3 newPos = cameraController.getCamera().getPosition();
        INFO("LIGHT1");
        lights[0]->setPosition(newPos);
    });
    setLightPosAction[1].init("setLight2", [&lights, &cameraController]() {
        glm::vec3 newPos = cameraController.getCamera().getPosition();
        INFO("LIGHT2");
        lights[1]->setPosition(newPos);
    });
    BindingsManager::bind(Button::left, "setLight1");
    BindingsManager::bind(Button::right, "setLight2");

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
        m_renderer.renderWorld(camera, world);

        // Render UI
        Renderer2D::beginScene(uiCamera);
        Renderer2D::drawString({0.0f, m_window.getHeight() - font.getHeight(), 0.5f}, std::format("{:.1f}", 1000.0f/smoothFrameTime) + "fps", font, {0.0f, 1.0f, 0.0f, 1.0f});
        Renderer2D::drawString(
            {0.0f, m_window.getHeight() - font.getHeight() * 2.0f, 0.5f},
            "XYZ: " + std::format("{:.2f}", cameraController.getCamera().getPosition().x)
                    + ", " + std::format("{:.2f}", cameraController.getCamera().getPosition().y)
                    + ", " + std::format("{:.2f}", cameraController.getCamera().getPosition().z),
            font,
            {1.0f, 1.0f, 1.0f, 1.0f}
        );
#define ENABLE_SHOW_DEPTH_TEXTURES
#ifdef ENABLE_SHOW_DEPTH_TEXTURES
        float height = 120.0f;
        for (unsigned int i = 0; i < m_renderer.m_shadowMapLevels.size(); i++)
        {
            const std::shared_ptr<Texture> &shadowMap = m_renderer.m_shadowMapLevels[i].framebuffer->getDepthAttachment();
            Renderer2D::drawQuad({0.0f, height*i, 0.0f}, {height, height}, shadowMap);
        }
#endif // DEBUG
        Renderer2D::endScene();

        m_window.swapBuffers();
    }
}
