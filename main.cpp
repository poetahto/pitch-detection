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

struct SinWaveGeneratorState {
    double currentFrame {0.0};
    double frequency {440.0};
    double targetFrequency {440.0};
    double amplitude {0.1};
    double sampleRate {44100.0};
};

int SinWaveGenerator(const void*, void* output, unsigned long frameCount, const PaStreamCallbackTimeInfo*, PaStreamCallbackFlags, void* userData) {
    auto state = reinterpret_cast<SinWaveGeneratorState*>(userData);
    auto samples = reinterpret_cast<float*>(output);
    double k = 2.0 * SDL_PI_D * state->frequency;
    for (unsigned long sample = 0; sample < frameCount; sample++) {
        auto value = static_cast<float>(SDL_sin(k * (state->currentFrame++ / state->sampleRate)) * state->amplitude);

        // ignore this gross part, its to reset the state when the frequency changes so you dont hear a discontinuity.
        if (static_cast<unsigned long>(state->currentFrame) % static_cast<unsigned long>(state->sampleRate / state->frequency) == 0) {
            state->frequency = state->targetFrequency;
            state->currentFrame = 0;
            k = 2.0 * SDL_PI_D * state->frequency;
            value = static_cast<float>(SDL_sin(k * (state->currentFrame++ / state->sampleRate)) * state->amplitude);
        }
        *samples++ = value;
        *samples++ = value;
    }
    return paContinue;
}

int main(int, char**) {
    if (Pa_IsError(Pa_Initialize())) {
        return 1;
    }
    printf("Initialized %s\n", Pa_GetVersionInfo()->versionText);
    if (!SDL_Init(SDL_INIT_VIDEO)) {
        SDL_PrintError();
        Pa_Terminate();
        return 1;
    }
    SDL_Window* window = SDL_CreateWindow("Pitch Detector", 800, 600, SDL_WINDOW_HIDDEN | SDL_WINDOW_RESIZABLE);
    if (window == nullptr) {
        SDL_PrintError();
        SDL_Quit();
        Pa_Terminate();
        return 1;
    }
    SDL_Renderer* renderer = SDL_CreateRenderer(window, nullptr);
    SDL_SetRenderVSync(renderer, 1);

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
    PaDeviceIndex selectedInputDevice = Pa_GetDefaultInputDevice();
    PaDeviceIndex selectedOutputDevice = Pa_GetDefaultOutputDevice();

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

        if (ImGui::Begin("Audio System")) {

            const PaDeviceInfo* currentInputInfo = Pa_GetDeviceInfo(selectedInputDevice);
            const PaDeviceInfo* currentOutputInfo = Pa_GetDeviceInfo(selectedOutputDevice);
            ImGui::Text("Current Input Device: %s (%s)", currentInputInfo->name, Pa_GetDeviceInfo(currentInputInfo->hostApi)->name);
            ImGui::Text("Current Output Device: %s (%s)", currentOutputInfo->name, Pa_GetDeviceInfo(currentOutputInfo->hostApi)->name);

            static PaStream* sinStream {nullptr};
            static SinWaveGeneratorState state {};

            // editor for controlling the sin generator
            if (sinStream != nullptr && ImGui::Button("Stop Sin")) {
                Pa_StopStream(sinStream);
                Pa_CloseStream(sinStream);
                sinStream = nullptr;
            }

            if (sinStream == nullptr && ImGui::Button("Play Sin")) {
                PaStreamParameters parameters {};
                parameters.device = selectedOutputDevice;
                parameters.channelCount = 2;
                parameters.sampleFormat = paFloat32;
                parameters.hostApiSpecificStreamInfo = nullptr;
                parameters.suggestedLatency = currentOutputInfo->defaultLowOutputLatency;
                state.sampleRate = currentOutputInfo->defaultSampleRate;
                state.currentFrame = 0;
                Pa_OpenStream(&sinStream, nullptr, &parameters, state.sampleRate, paFramesPerBufferUnspecified, 0, SinWaveGenerator, &state);
                Pa_StartStream(sinStream);
            }

            static float volume {0.1};
            if (ImGui::SliderFloat("Volume", &volume, 0, 1)) {
                state.amplitude = static_cast<double>(volume);
            }
            static float frequency {440};
            if (ImGui::DragFloat("Frequency", &frequency)) {
                state.targetFrequency = frequency;
            }

            // editor for selecting the input and output devices
            for (PaHostApiIndex index = 0; index < Pa_GetHostApiCount(); index++) {
                const PaHostApiInfo* info = Pa_GetHostApiInfo(index);
                if (ImGui::TreeNode(info->name, "%s%s", info->name, index == Pa_GetDefaultHostApi() ? " (default)" : "")) {
                    for (PaDeviceIndex device = 0; device < info->deviceCount; device++) {
                        PaDeviceIndex d = Pa_HostApiDeviceIndexToDeviceIndex(index, device);
                        const PaDeviceInfo* deviceInfo = Pa_GetDeviceInfo(d);
                        bool isInputDevice = deviceInfo->maxInputChannels > 0;
                        bool isOutputDevice = deviceInfo->maxOutputChannels > 0;
                        ImGui::TreePush(deviceInfo->name);
                        ImGui::Selectable(deviceInfo->name, selectedInputDevice == d || selectedOutputDevice == d);
                        if (ImGui::IsItemHovered()) {
                            ImGui::SetTooltip("Device %i (%i) [%i input, %i output]", device, d, deviceInfo->maxInputChannels, deviceInfo->maxOutputChannels);
                        }
                        if (ImGui::BeginPopupContextItem()) {
                            if (isInputDevice && ImGui::Selectable("Set Input Device")) {
                                selectedInputDevice = d;
                            }
                            if (isOutputDevice && ImGui::Selectable("Set Output Device")) {
                                selectedOutputDevice = d;
                            }
                            ImGui::EndPopup();
                        }
                        ImGui::TreePop();
                    }
                    ImGui::TreePop();
                }
            }
        }
        ImGui::End();

        ImGui::Render();
        SDL_RenderClear(renderer);
        ImGui_ImplSDLRenderer3_RenderDrawData(ImGui::GetDrawData(), renderer);
        SDL_RenderPresent(renderer);

    }
    ImGui_ImplSDLRenderer3_Shutdown();
    ImGui_ImplSDL3_Shutdown();
    ImGui::DestroyContext();
    SDL_DestroyWindow(window);
    SDL_Quit();
    if (Pa_IsError(Pa_Terminate())) {
        // idk how we fail to terminate lol
        return 1;
    }
    return 0;
}