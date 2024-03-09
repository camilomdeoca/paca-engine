#include "App.hpp"
#include "Input.hpp"
#include "OrthoCameraController.hpp"
#include "../opengl/gl.hpp"
#include "Renderer2D.hpp"
#include "opengl/Texture.hpp"

#include <SDL2/SDL.h>
#include <cstdio>
#include <format>
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
    Renderer2D::init();
}

void App::run()
{
    OrthoCameraController cameraController(16.0f / 9.0f);
    OrthoCamera uiCamera(0.0f, 1600.0f, 0.0f, 900.0f);
    std::shared_ptr<Texture> bigImageTexture = std::make_shared<Texture>("assets/textures/big.jpg");

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

        /* Draw */
        GL::setClearColor({0.1f, 0.1f, 0.15f, 1.0f});
        GL::clear();

        Renderer2D::beginScene(cameraController.getCamera());
        Renderer2D::drawQuad(
                { -(float)bigImageTexture->getWidth()/(float)bigImageTexture->getHeight(), -1.0f, 0.1f },
                { (float)bigImageTexture->getWidth()/(float)bigImageTexture->getHeight(), 1.0f },
                bigImageTexture,
                { 1.0f, 1.0f, 1.0f, 1.0f });

        for (int y = 0; y < 20; y++)
        {
            for (int x = 0; x < 100; x++)
            {
                Renderer2D::drawQuad(
                { x, y, 0.1f },
                { (float)bigImageTexture->getWidth()/(float)bigImageTexture->getHeight(), 1.0f },
                bigImageTexture,
                { 1.0f, 1.0f, 1.0f, 1.0f });

            }
        }
        Renderer2D::drawString({0.0f, 25.0f, 0.2f}, "pspspssspspsspspssspssps", {0.9f, 0.9f, 1.0f, 1.0f});
        Renderer2D::endScene();
        Renderer2D::beginScene(uiCamera);
        Renderer2D::drawString({0.0f, 900.0f-24.0f, 0.5f}, std::format("{:.1f}", 1000.0f/smoothFrameTime) + "fps", {0.0f, 1.0f, 0.0f, 1.0f});
        Renderer2D::endScene();

        window.swapBuffers();
    }
}
