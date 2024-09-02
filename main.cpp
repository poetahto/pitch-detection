#include <cstdio>
#include <portaudio.h>
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <imgui.h>
#include <imgui_impl_sdl3.h>
#include <imgui_impl_sdlrenderer3.h>

bool Pa_IsError(PaError errorCode) {
    if (errorCode != paNoError) {
        printf("PortAudio Error: %s\n", Pa_GetErrorText(errorCode));
        return true;
    }
    return false;
}

void SDL_PrintError() {
    printf("SDL Error: %s\n", SDL_GetError());
}

int main(int argc, char** argv) {
    if (Pa_IsError(Pa_Initialize())) {
        return 1;
    }
    printf("Initialized %s\n", Pa_GetVersionInfo()->versionText);
    if (!SDL_Init(SDL_INIT_VIDEO)) {
        SDL_PrintError();
        Pa_Terminate();
        return 1;
    }
    SDL_Window* window = SDL_CreateWindow("Pitch Detector", 800, 600, SDL_WINDOW_HIDDEN);
    if (window == nullptr) {
        SDL_PrintError();
        SDL_Quit();
        Pa_Terminate();
        return 1;
    }
    SDL_Renderer* renderer = SDL_CreateRenderer(window, nullptr);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;
    ImGui::StyleColorsDark();
    ImGui_ImplSDL3_InitForSDLRenderer(window, renderer);
    ImGui_ImplSDLRenderer3_Init(renderer);

    SDL_ShowWindow(window);
    bool isRunning {true};
    while (isRunning) {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            ImGui_ImplSDL3_ProcessEvent(&event);
            switch (event.type) {
                case SDL_EVENT_QUIT: {
                    isRunning = false;
                    break;
                }
            }
        }
        ImGui_ImplSDLRenderer3_NewFrame();
        ImGui_ImplSDL3_NewFrame();
        ImGui::NewFrame();

        // actual logic here
        ImGui::ShowDemoWindow();

        ImGui::Render();
        SDL_RenderClear(renderer); // todo: error check
        ImGui_ImplSDLRenderer3_RenderDrawData(ImGui::GetDrawData(), renderer);
        SDL_RenderPresent(renderer); // todo: error check

    }
    ImGui_ImplSDLRenderer3_Shutdown();
    ImGui_ImplSDL3_Shutdown();
    ImGui::DestroyContext();
    SDL_DestroyWindow(window);
    SDL_Quit();
    if (Pa_IsError(Pa_Terminate())) {
        return 1;
    }
    return 0;
}