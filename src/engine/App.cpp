#include "App.hpp"
#include "Input.hpp"
#include "OrthoCameraController.hpp"
#include "../opengl/gl.hpp"
#include "Renderer2D.hpp"
#include "opengl/Texture.hpp"

#include <SDL2/SDL.h>
#include <cstdio>
#include <memory>

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
    std::shared_ptr<Texture> texture = std::make_shared<Texture>("assets/textures/big.jpg");

    float lastFrameTime = SDL_GetTicks();
    while (true) {
        float time = SDL_GetTicks();
        float timeDelta = time - lastFrameTime;
        lastFrameTime = time;

        Input::processInput();

        cameraController.onUpdate(timeDelta);

        /* Draw */
        GL::setClearColor({0.1f, 0.1f, 0.15f, 1.0f});
        GL::clear();

        Renderer2D::beginScene(cameraController.getCamera());
        Renderer2D::drawQuad({ 0.0f, 0.0f, 0.0f }, { 1.0f, 1.0f }, texture, { 1.0f, 1.0f, 1.0f, 1.0f });
        Renderer2D::drawQuad({ 0.5f, 0.5f, 0.0f }, { 1.0f, 0.5f }, { 0.8f, 0.6f, 0.3f, 1.0f });
        constexpr int count = 15;
        for (int y = 0; y < count; y++)
        {
            for (int x = 0; x < count; x++)
            {
                Renderer2D::drawQuad({ x, y, 0.0f }, { 0.8f, 0.8f }, texture, { 0.3f, 0.3f, 0.8f, 1.0f });
            }
        }
        Renderer2D::endScene();

        window.swapBuffers();
    }
}
