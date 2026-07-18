#define NOMINMAX
#include "base/log.h"
#include "vk_surface.h"
#include "vk_utils.h"
#include "../engine/engine_state.h"

#include <vulkan/vulkan.h>
#include <windows.h>

bool createSurface() {
    VkWin32SurfaceCreateInfoKHR createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
    createInfo.hwnd = g_hwnd;
    createInfo.hinstance = GetModuleHandle(nullptr);

    VK_CHECK(vkCreateWin32SurfaceKHR(g_instance, &createInfo, nullptr, &g_surface));
    return true;
}