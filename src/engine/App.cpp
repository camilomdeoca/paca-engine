#include "App.hpp"
#include "Input.hpp"
#include "engine/OrthoCamera.hpp"
#include "opengl/gl.hpp"
#include "Renderer2D.hpp"
#include "engine/PerspectiveCameraController.hpp"
#include "engine/Renderer.hpp"

#include <SDL2/SDL.h>
#include <cstdio>
#include <cstdlib>
#include <format>
#include <glm/glm.hpp>
#include <memory>
#include <string>

App::App()
{}

App::~App()
{
    SDL_Quit();
}

void App::init(std::string title)
{
    if (SDL_Init(SDL_INIT_EVERYTHING) < 0) {
        fprintf(stderr, "Error initializing SDL: %s\n", SDL_GetError());
        exit(1);
    }

    m_window.create(title);

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

    RendererParameters rendererParams;
    rendererParams.width = m_window.getWidth();
    rendererParams.height = m_window.getHeight();

    GL::init();
    Input::init();
    Renderer::init(rendererParams);
    Renderer2D::init();
}

void App::run()
{
    Input::restrainMouseToWindow(true);
    PerspectiveCameraController cameraController((float)m_window.getWidth() / m_window.getHeight(), 90.0f);
    cameraController.resumeControl();
    OrthoCamera uiCamera(0.0f, m_window.getWidth(), 0.0f, m_window.getHeight());
    Font font("assets/fonts/DejaVuSansFontAtlas.png", "assets/fonts/DejaVuSansFontAtlas.fntat");

    Model mainModel("assets/meshes/f1961_33-part_01-laser-ortery_texture-150k-4096-gltf_std/f1961_33-part_01-laser-ortery_texture-150k-4096.gltf");
    mainModel.setScale(glm::vec3(10.0f));
    Model plane("assets/meshes/plane/plane.gltf");
    plane.setPosition({0.0f, -3.0f, 0.0f});
    std::shared_ptr<Model> lightBulb = std::make_shared<Model>("assets/meshes/light/scene.gltf");
    lightBulb->setPosition({3.0f, 4.0f, -3.0f});
    lightBulb->setRotation({-90.0f, 0.0f, 0.0f});
    lightBulb->setScale(glm::vec3(3.0f));

    std::vector<std::shared_ptr<Light>> lights;
    std::shared_ptr<Light> light = std::make_shared<Light>(glm::vec3(3.0f, 4.0f, -3.0f), glm::vec3(1.0f, 1.0f, 1.0f), 1.0f, 0.0075f, lightBulb);
    lights.push_back(light);

    Input::addKeyPressCallback([light](KeyPressEvent &event) {
        light->setIntensity(light->getIntensity() - 0.1);
        printf("intensity = %f\n", light->getIntensity());
    }, Key::down);
    Input::addKeyPressCallback([light](KeyPressEvent &event) {
        light->setIntensity(light->getIntensity() + 0.1);
        printf("intensity = %f\n", light->getIntensity());
    }, Key::up);
    Input::addKeyPressCallback([light](KeyPressEvent &event) {
        light->setAttenuation(light->getAttenuation() * 1.05);
        printf("attenuation = %f\n", light->getAttenuation());
    }, Key::left);
    Input::addKeyPressCallback([light](KeyPressEvent &event) {
        light->setAttenuation(light->getAttenuation() * 0.95);
        printf("attenuation = %f\n", light->getAttenuation());
    }, Key::right);
    Input::addMouseButtonPressCallback([&light, &cameraController](ButtonPressEvent &event) {
        glm::vec3 newPos = cameraController.getCamera().getPosition();
        light->setPosition(newPos);
    }, Button::left);

    Input::addKeyPressCallback([&cameraController](KeyPressEvent &event) {
        static bool val = true;
        val = !val;
        Input::restrainMouseToWindow(val);
        if (!val)
            cameraController.pauseControl();
        else
            cameraController.resumeControl();
    }, Key::esc);

    float lastFrameTime = SDL_GetTicks();
    while (true) {
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
        Renderer::drawModel(plane);
        Renderer::drawModel(mainModel);
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
