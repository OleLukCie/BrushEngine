cmake_minimum_required(VERSION 3.14)

# BrushEngine
set(RENDER_SOURCES
    ${CMAKE_CURRENT_LIST_DIR}/blend_mode.cpp
    ${CMAKE_CURRENT_LIST_DIR}/brush_renderer.cpp
    ${CMAKE_CURRENT_LIST_DIR}/canvas.cpp
    ${CMAKE_CURRENT_LIST_DIR}/gpu_brush_renderer.cpp
    ${CMAKE_CURRENT_LIST_DIR}/gpu_canvas.cpp
    ${CMAKE_CURRENT_LIST_DIR}/layer.cpp
    ${CMAKE_CURRENT_LIST_DIR}/render_target.cpp
    ${CMAKE_CURRENT_LIST_DIR}/texture.cpp
    ${CMAKE_CURRENT_LIST_DIR}/ui_panel.cpp
)

set(RENDER_HEADERS
    ${CMAKE_CURRENT_LIST_DIR}/blend_mode.h
    ${CMAKE_CURRENT_LIST_DIR}/brush_renderer.h
    ${CMAKE_CURRENT_LIST_DIR}/canvas.h
    ${CMAKE_CURRENT_LIST_DIR}/gpu_brush_renderer.h
    ${CMAKE_CURRENT_LIST_DIR}/gpu_canvas.h
    ${CMAKE_CURRENT_LIST_DIR}/layer.h
    ${CMAKE_CURRENT_LIST_DIR}/render_target.h
    ${CMAKE_CURRENT_LIST_DIR}/texture.h
    ${CMAKE_CURRENT_LIST_DIR}/ui_panel.h
)

add_library(render STATIC ${RENDER_SOURCES} ${RENDER_HEADERS})

target_include_directories(render PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/3rdparty/imgui
    ${CMAKE_SOURCE_DIR}/3rdparty/imgui/backends
    ${CMAKE_SOURCE_DIR}/src/vulkan
)

target_link_libraries(render PUBLIC
    base
    imgui
    Vulkan::Vulkan
)

