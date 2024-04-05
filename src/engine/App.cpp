#include "App.hpp"
#include "Input.hpp"
#include "engine/Action.hpp"
#include "engine/OrthoCamera.hpp"
#include "engine/ResourceManager.hpp"
#include "opengl/gl.hpp"
#include "Renderer2D.hpp"
#include "engine/PerspectiveCameraController.hpp"
#include "engine/Renderer.hpp"

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

    RendererParameters rendererParams;
    rendererParams.width = m_window.getWidth();
    rendererParams.height = m_window.getHeight();

    GL::init();
    Input::init();
    BindingsManager::init();
    Renderer::init(rendererParams);
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

    ResourceManager::addMaterial("assets/materials/thing.pmat");
    ResourceManager::addMaterial("assets/materials/plane.pmat");
    ResourceManager::addMaterial("assets/materials/lightBulb.pmat");

    std::shared_ptr<Model> mainModel = ResourceManager::addModel("assets/models/thing.pmdl");
    mainModel->setScale(glm::vec3(10.0f));
    std::shared_ptr<Model> plane = ResourceManager::addModel("assets/models/plane.pmdl");
    plane->setPosition({0.0f, -3.0f, 0.0f});
    std::shared_ptr<Model> lightBulb = ResourceManager::addModel("assets/models/lightBulb.pmdl");
    lightBulb->setPosition({3.0f, 4.0f, -3.0f});
    lightBulb->setRotation({-90.0f, 0.0f, 0.0f});
    lightBulb->setScale(glm::vec3(3.0f));

    std::vector<std::shared_ptr<Light>> lights;
    std::shared_ptr<Light> light = std::make_shared<Light>(glm::vec3(3.0f, 4.0f, -3.0f), glm::vec3(1.0f, 1.0f, 1.0f), 1.0f, 0.0075f, lightBulb);
    lights.push_back(light);

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
    Action setLightPosAction("setLight", [&light, &cameraController]() {
        glm::vec3 newPos = cameraController.getCamera().getPosition();
        light->setPosition(newPos);
    });
    BindingsManager::bindKeyToAction(Key::left, "setLight");

    //Input::addKeyPressCallback([&cameraController](KeyPressEvent &event) {
    //    static bool val = true;
    //    val = !val;
    //    Input::restrainMouseToWindow(val);
    //    if (!val)
    //        cameraController.pauseControl();
    //    else
    //        cameraController.resumeControl();
    //}, Key::esc);

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

        RenderEnvironment environment;
        environment.pointLights = &lights;

        Renderer::beginScene(cameraController.getCamera(), environment);
        Renderer::drawModel(*plane);
        Renderer::drawModel(*mainModel);
        Renderer::endScene();

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

        m_window.swapBuffers();
    }
}
