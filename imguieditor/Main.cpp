#include "App.hpp"

int main(int argc, char *argv[])
{
    App app;
    app.init();

    app.run();

    return 0;
    //if (SDL_Init(SDL_INIT_EVERYTHING) < 0) {
    //    ERROR("Error initializing SDL: {}", SDL_GetError());
    //    ASSERT(false);
    //}

    //Window window;
    //window.create("Editor", 1280, 720);

    //IMGUI_CHECKVERSION();
    //ImGui::CreateContext();
    //ImGuiIO &io = ImGui::GetIO();
    //(void)io;
    //io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    //io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;

    //ImGui::StyleColorsDark();

    //ImGui_ImplSDL2_InitForOpenGL(window.getSDLWindow(), window.getSDLGLContext());
    //ImGui_ImplOpenGL3_Init("#version 450 core");

    //bool showDemo = true;
    //bool running = true;
    //EventReceiver exitEventReceiver;
    //exitEventReceiver.setEventHandler([&running] (const Event &event) {
    //    switch (event.getType()) {
    //    case EventType::exit:
    //        running = false;
    //        break;
    //    default: break;
    //    }
    //});
    //exitEventReceiver.setEventsMask(EventMask::exit);

    //while(running)
    //{
    //    SDL_Event event;
    //    while (SDL_PollEvent(&event))
    //    {
    //        // If event is for imgui dont process it
    //        if (ImGui_ImplSDL2_ProcessEvent(&event))
    //            continue;

    //        Input::processSDLEvent(event);
    //    }
    //    if (SDL_GetWindowFlags(window.getSDLWindow()) & SDL_WINDOW_MINIMIZED)
    //    {
    //        SDL_Delay(10);
    //        continue;
    //    }

    //    ImGui_ImplOpenGL3_NewFrame();
    //    ImGui_ImplSDL2_NewFrame();
    //    ImGui::NewFrame();

    //    if (showDemo)
    //    {
    //        ImGui::ShowDemoWindow(&showDemo);
    //    }

    //    ImGui::Render();
    //    GL::clear();
    //    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    //    window.swapBuffers();
    //}

    //ImGui_ImplOpenGL3_Shutdown();
    //ImGui_ImplSDL2_Shutdown();
    //ImGui::DestroyContext();

    //return 0;
}
