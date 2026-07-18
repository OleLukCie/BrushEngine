cmake_minimum_required(VERSION 3.14)
project(BrushEngine)

if(NOT CMAKE_SIZEOF_VOID_P EQUAL 8)
    message(FATAL_ERROR "This project requires 64-bit build. Please use -A x64 with Visual Studio generator.")
endif()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)


if(MSVC)
    add_compile_options(/utf-8)
    add_compile_options(/MP)
    # set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
endif()


find_package(Vulkan REQUIRED)

if(NOT Vulkan_FOUND)
    message(FATAL_ERROR 
        "Vulkan SDK not found! Please install from:\n"
        "https://vulkan.lunarg.com/sdk/home#windows\n"
        "Make sure VULKAN_SDK environment variable is set."
    )
endif()

message(STATUS "Vulkan SDK found: ${Vulkan_INCLUDE_DIRS}")
message(STATUS "Vulkan libraries: ${Vulkan_LIBRARIES}")

add_subdirectory(3rdparty)
add_subdirectory(src/base)
add_subdirectory(src/render)
add_subdirectory(src/sys)
add_subdirectory(src/tool)
add_subdirectory(src/engine)
add_subdirectory(src/vulkan)


add_executable(BrushEngine WIN32 src/main.cpp)

target_link_libraries(BrushEngine PRIVATE
    base
    render
    sys
    tool
    engine
    vulkan_rhi
    Vulkan::Vulkan
)

set_target_properties(BrushEngine PROPERTIES
    WIN32_EXECUTABLE TRUE
    LINK_FLAGS "/SUBSYSTEM:WINDOWS /ENTRY:mainCRTStartup"
)