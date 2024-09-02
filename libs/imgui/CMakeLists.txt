project(imgui)
add_library(imgui
        imgui.cpp
        imgui_demo.cpp
        imgui_tables.cpp
        imgui_draw.cpp
        imgui_widgets.cpp
        imgui_impl_sdl3.cpp
        imgui_impl_sdlrenderer3.cpp
)
target_link_libraries(imgui PUBLIC SDL3::SDL3)
target_include_directories(imgui PUBLIC ${PROJECT_SOURCE_DIR})