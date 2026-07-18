cmake_minimum_required(VERSION 3.14)

# BrushEngine
set(SYS_SOURCES
    ${CMAKE_CURRENT_LIST_DIR}/brush.cpp
    ${CMAKE_CURRENT_LIST_DIR}/brush_circle.cpp
    ${CMAKE_CURRENT_LIST_DIR}/brush_marker.cpp
    ${CMAKE_CURRENT_LIST_DIR}/brush_pencil.cpp
    ${CMAKE_CURRENT_LIST_DIR}/brush_watercolor.cpp
    ${CMAKE_CURRENT_LIST_DIR}/input_sample.cpp
    ${CMAKE_CURRENT_LIST_DIR}/shortcut.cpp
    ${CMAKE_CURRENT_LIST_DIR}/tool_manager.cpp
    ${CMAKE_CURRENT_LIST_DIR}/ui_manager.cpp
)

set(SYS_HEADERS
    ${CMAKE_CURRENT_LIST_DIR}/brush.h
    ${CMAKE_CURRENT_LIST_DIR}/brush_circle.h
    ${CMAKE_CURRENT_LIST_DIR}/brush_marker.h
    ${CMAKE_CURRENT_LIST_DIR}/brush_pencil.h
    ${CMAKE_CURRENT_LIST_DIR}/brush_watercolor.h
    ${CMAKE_CURRENT_LIST_DIR}/input_sample.h
    ${CMAKE_CURRENT_LIST_DIR}/shortcut.h
    ${CMAKE_CURRENT_LIST_DIR}/tool_manager.h
    ${CMAKE_CURRENT_LIST_DIR}/tool_type.h
    ${CMAKE_CURRENT_LIST_DIR}/ui_manager.h
)

add_library(sys STATIC ${SYS_SOURCES} ${SYS_HEADERS})

target_include_directories(sys PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/3rdparty/imgui
    ${CMAKE_SOURCE_DIR}/3rdparty/imgui/backends
    ${CMAKE_SOURCE_DIR}/src/render
)

target_link_libraries(sys PUBLIC
    base
    render
    imgui
    Vulkan::Vulkan
)

