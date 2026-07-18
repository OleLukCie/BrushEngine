set(VULKAN_SOURCES
    ${CMAKE_CURRENT_LIST_DIR}/vk_brush_pipeline.cpp
    ${CMAKE_CURRENT_LIST_DIR}/vk_buffer.cpp
    ${CMAKE_CURRENT_LIST_DIR}/vk_canvas_tex.cpp
    ${CMAKE_CURRENT_LIST_DIR}/vk_commands.cpp
    ${CMAKE_CURRENT_LIST_DIR}/vk_composite_pipeline.cpp
    ${CMAKE_CURRENT_LIST_DIR}/vk_core.cpp
    ${CMAKE_CURRENT_LIST_DIR}/vk_device.cpp
    ${CMAKE_CURRENT_LIST_DIR}/vk_imageview.cpp
    ${CMAKE_CURRENT_LIST_DIR}/vk_instance.cpp
    ${CMAKE_CURRENT_LIST_DIR}/vk_pipeline.cpp
    ${CMAKE_CURRENT_LIST_DIR}/vk_renderpass.cpp
    ${CMAKE_CURRENT_LIST_DIR}/vk_surface.cpp
    ${CMAKE_CURRENT_LIST_DIR}/vk_swapchain.cpp
    ${CMAKE_CURRENT_LIST_DIR}/vk_sync.cpp
    ${CMAKE_CURRENT_LIST_DIR}/vk_utils.cpp
)

set(VULKAN_HEADERS
    ${CMAKE_CURRENT_LIST_DIR}/vk_brush_pipeline.h
    ${CMAKE_CURRENT_LIST_DIR}/vk_buffer.h
    ${CMAKE_CURRENT_LIST_DIR}/vk_canvas_tex.h
    ${CMAKE_CURRENT_LIST_DIR}/vk_commands.h
    ${CMAKE_CURRENT_LIST_DIR}/vk_composite_pipeline.h
    ${CMAKE_CURRENT_LIST_DIR}/vk_core.h
    ${CMAKE_CURRENT_LIST_DIR}/vk_device.h
    ${CMAKE_CURRENT_LIST_DIR}/vk_imageview.h
    ${CMAKE_CURRENT_LIST_DIR}/vk_instance.h
    ${CMAKE_CURRENT_LIST_DIR}/vk_pipeline.h
    ${CMAKE_CURRENT_LIST_DIR}/vk_renderpass.h
    ${CMAKE_CURRENT_LIST_DIR}/vk_surface.h
    ${CMAKE_CURRENT_LIST_DIR}/vk_swapchain.h
    ${CMAKE_CURRENT_LIST_DIR}/vk_sync.h
    ${CMAKE_CURRENT_LIST_DIR}/vk_utils.h
)

add_library(vulkan_rhi STATIC ${VULKAN_SOURCES} ${VULKAN_HEADERS})

target_include_directories(vulkan_rhi PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/3rdparty/imgui
    ${Vulkan_INCLUDE_DIRS}
)

target_link_libraries(vulkan_rhi PUBLIC
    engine
    Vulkan::Vulkan
)

target_compile_definitions(vulkan_rhi PRIVATE
    VK_USE_PLATFORM_WIN32_KHR
)

