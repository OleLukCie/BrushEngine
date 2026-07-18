# BrushEngine
set(ENGINE_SOURCES
    ${CMAKE_CURRENT_LIST_DIR}/engine_init.cpp
    ${CMAKE_CURRENT_LIST_DIR}/engine_state.cpp
    ${CMAKE_CURRENT_LIST_DIR}/render_loop.cpp
    ${CMAKE_CURRENT_LIST_DIR}/tool_switcher.cpp
    ${CMAKE_CURRENT_LIST_DIR}/window_proc.cpp
)

set(ENGINE_HEADERS
    ${CMAKE_CURRENT_LIST_DIR}/engine_init.h
    ${CMAKE_CURRENT_LIST_DIR}/engine_state.h
    ${CMAKE_CURRENT_LIST_DIR}/render_loop.h
    ${CMAKE_CURRENT_LIST_DIR}/tool_switcher.h
    ${CMAKE_CURRENT_LIST_DIR}/window_proc.h
)

add_library(engine STATIC ${ENGINE_SOURCES} ${ENGINE_HEADERS})

target_include_directories(engine PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/3rdparty/imgui
    ${Vulkan_INCLUDE_DIRS}
)

target_link_libraries(engine PUBLIC
    base
    render
    sys
    tool
    Vulkan::Vulkan
)

