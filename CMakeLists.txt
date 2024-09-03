cmake_minimum_required(VERSION 3.15)
project(pitch_detection)
set(BUILD_SHARED_LIBS OFF)
set(PA_USE_ASIO OFF) # asio is cool, but messes w audio on initialization :( (thus is optional)
add_subdirectory(libs/portaudio)
add_subdirectory(libs/SDL)
add_subdirectory(libs/imgui)
set(SOURCES
    main.cpp
)
add_executable(pitch_detection ${SOURCES})
target_link_libraries(pitch_detection PUBLIC PortAudio imgui SDL3::SDL3)
