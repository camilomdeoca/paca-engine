#include "App.hpp"
#include "Input.hpp"
#include "OrthoCameraController.hpp"
#include "../opengl/gl.hpp"
#include "Renderer2D.hpp"
#include "engine/PerspectiveCameraController.hpp"
#include "engine/Renderer.hpp"
#include "opengl/Texture.hpp"

#include <SDL2/SDL.h>
#include <cstdio>
#include <cstdlib>
#include <format>
#include <glm/fwd.hpp>
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

    window.create(title);

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

    GL::init();
    Input::init();
    Renderer::init();
    Renderer2D::init();
}

void App::run()
{
    Input::restrainMouseToWindow(true);
    PerspectiveCameraController cameraController(1600.0f / 900.0f, 90.0f);
    //OrthoCameraController cameraController(1600.0f / 900.0f);
    OrthoCamera uiCamera(0.0f, 1600.0f, 0.0f, 900.0f);
    std::shared_ptr<Texture> bigImageTexture = std::make_shared<Texture>("assets/textures/cat1.png");
    std::shared_ptr<Texture> bigImageTexture2 = std::make_shared<Texture>("assets/textures/cat2.png");
    Font font("assets/fonts/DejaVuSansFontAtlas.png", "assets/fonts/DejaVuSansFontAtlas.fntat");
    bigImageTexture->setInterpolate(false);
    bigImageTexture2->setInterpolate(false);

    //Mesh teapot("assets/meshes/teapot1.obj");
    Model mainModel("assets/meshes/backpack/backpack.obj");
    Model plane("assets/meshes/plane/plane.gltf");
    plane.setPosition({0.0f, -3.0f, 0.0f});
    //Model mainModel("assets/meshes/cube/scene.gltf");
    //mainModel.setScale(glm::vec3(0.005f));
    Model lightBulb("assets/meshes/light/scene.gltf");
    lightBulb.setPosition({3.0f, 4.0f, -3.0f});
    lightBulb.setRotation({-90.0f, 0.0f, 0.0f});
    lightBulb.setScale(glm::vec3(10.0f));

    std::vector<std::shared_ptr<Light>> lights;
    std::shared_ptr<Light> light = std::make_shared<Light>(glm::vec3(3.0f, 4.0f, -3.0f), glm::vec3(1.0f, 1.0f, 1.0f), 1.0f, 0.0075f);
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
        light->setAttenuation(light->getAttenuation() + 0.0005);
        printf("attenuation = %f\n", light->getAttenuation());
    }, Key::left);
    Input::addKeyPressCallback([light](KeyPressEvent &event) {
        light->setAttenuation(light->getAttenuation() - 0.0005);
        printf("attenuation = %f\n", light->getAttenuation());
    }, Key::right);


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
        //glm::vec3 rotation = backpack.getRotation();
        //rotation.y = 15.0f * sin(time * 0.01f);
        //rotation.x = 15.0f *sin(time * 0.005f);
        //backpack.setRotation(rotation);

        //glm::vec3 position = backpack.getPosition();
        //position.y = 0.3f * sin(time * 0.01f);
        //backpack.setPosition(position);
        
        // Rotate Light
        static float distance = 3.0f;
        glm::vec3 newLightPos(distance * sin(time*0.002f), 2.0f, distance * cos(time*0.002f));
        //distance += 0.001 * timeDelta;
        light->setPosition(newLightPos);
        lightBulb.setPosition(newLightPos);

        /* Draw */
        GL::setClearColor({0.1f, 0.1f, 0.15f, 1.0f});
        GL::clear();

        RenderEnvironment environment;
        environment.pointLights = &lights;

        Renderer::beginScene(cameraController.getCamera(), environment);
        //Renderer::drawMesh(teapot);
        Renderer::drawModel(plane);
        Renderer::drawModel(mainModel);
        Renderer::drawModel(lightBulb);
        Renderer::endScene();

        //Renderer2D::beginScene(cameraController.getCamera());
        //for (int y = 0; y < 20; y++)
        //{
        //    for (int x = 0; x < 100; x++)
        //    {
        //        std::shared_ptr<Texture> &texture = (x + y) % 2 ? bigImageTexture : bigImageTexture2;
        //        Renderer2D::drawQuad(
        //            { x, y, (x + y) % 2 == 0 ? 0.0f : 0.2f },
        //            { (float)texture->getWidth()/(float)texture->getHeight(), 1.0f },
        //            texture,
        //            { 1.0f, 1.0f, 1.0f, 1.0f });

        //    }
        //}
        //Renderer2D::drawString({0.0f, 25.0f, 0.2f}, "pspspssspspsspspssspssps", font, {0.9f, 0.9f, 1.0f, 1.0f}, 0.01f);
        //Renderer2D::endScene();
        Renderer2D::beginScene(uiCamera);
        Renderer2D::drawString({0.0f, 900.0f-24.0f, 0.5f}, std::format("{:.1f}", 1000.0f/smoothFrameTime) + "fps", font, {0.0f, 1.0f, 0.0f, 1.0f});
        Renderer2D::drawString(
            {0.0f, 900.0f-24.0f*2, 0.5f},
            "X: " + std::format("{:.2f}", cameraController.getCamera().getPosition().x),
            font,
            {1.0f, 1.0f, 1.0f, 1.0f}
        );
        Renderer2D::drawString(
            {0.0f, 900.0f-24.0f*3, 0.5f},
            "Y: " + std::format("{:.2f}", cameraController.getCamera().getPosition().y),
            font,
            {1.0f, 1.0f, 1.0f, 1.0f}
        );
        Renderer2D::drawString(
            {0.0f, 900.0f-24.0f*4, 0.5f},
            "Z: " + std::format("{:.2f}", cameraController.getCamera().getPosition().z),
            font,
            {1.0f, 1.0f, 1.0f, 1.0f}
        );
        Renderer2D::endScene();

        window.swapBuffers();
    }
}
