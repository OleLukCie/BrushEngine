# 3rdparty/CMakeLists.txt
cmake_minimum_required(VERSION 3.14)

# ============================================
# ImGui (docking branch)
# ============================================
add_library(imgui STATIC
    imgui/imgui.cpp
    imgui/imgui_draw.cpp
    imgui/imgui_tables.cpp
    imgui/imgui_widgets.cpp
    imgui/imgui_demo.cpp
    imgui/backends/imgui_impl_win32.cpp
    imgui/backends/imgui_impl_vulkan.cpp
)

target_include_directories(imgui PUBLIC 
    ${CMAKE_CURRENT_SOURCE_DIR}/imgui
    ${CMAKE_CURRENT_SOURCE_DIR}/imgui/backends
)

target_link_libraries(imgui PUBLIC Vulkan::Vulkan)

# ============================================
# GLM (header-only)
# ============================================
add_library(glm INTERFACE)
target_include_directories(glm INTERFACE glm)

# ============================================
# stb (header-only)
# ============================================
add_library(stb INTERFACE)
target_include_directories(stb INTERFACE stb)

# ============================================
# spdlog
# ============================================
# header-only
add_library(spdlog INTERFACE)
target_include_directories(spdlog INTERFACE 
    ${CMAKE_CURRENT_SOURCE_DIR}/spdlog/include
)


# add_subdirectory(spdlog)
# target_link_libraries(spdlog PUBLIC spdlog::spdlog)