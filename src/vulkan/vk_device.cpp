#define NOMINMAX
#include "base/log.h"
#include "vk_device.h"
#include "vk_utils.h"
#include "../engine/engine_state.h"

#include <iostream>
#include <set>
#include <optional>
#include <map>

bool pickPhysicalDevice() {
    uint32_t deviceCount = 0;
    vkEnumeratePhysicalDevices(g_instance, &deviceCount, nullptr);
    if (deviceCount == 0) return false;

    std::vector<VkPhysicalDevice> devices(deviceCount);
    vkEnumeratePhysicalDevices(g_instance, &deviceCount, devices.data());

    std::multimap<int, VkPhysicalDevice> candidates;

    for (const auto& device : devices) {
        VkPhysicalDeviceProperties props;
        VkPhysicalDeviceFeatures features;
        vkGetPhysicalDeviceProperties(device, &props);
        vkGetPhysicalDeviceFeatures(device, &features);

        int score = 0;

        if (props.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) {
            score += 1000;
        } else if (props.deviceType == VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU) {
            score += 500;
        } else if (props.deviceType == VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU) {
            score += 200;
        }

        uint32_t queueFamilyCount = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);
        std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
        vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

        bool hasGraphics = false;
        bool hasPresent = false;
        std::optional<uint32_t> graphicsFamily;
        std::optional<uint32_t> presentFamily;

        for (uint32_t i = 0; i < queueFamilyCount; ++i) {
            if (queueFamilies[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
                hasGraphics = true;
                if (!graphicsFamily.has_value()) graphicsFamily = i;
            }

            VkBool32 presentSupport = false;
            vkGetPhysicalDeviceSurfaceSupportKHR(device, i, g_surface, &presentSupport);
            if (presentSupport) {
                hasPresent = true;
                if (!presentFamily.has_value()) presentFamily = i;
            }

            if ((queueFamilies[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) && presentSupport) {
                graphicsFamily = i;
                presentFamily = i;
                break;
            }
        }

        if (!hasGraphics) continue;

        score += static_cast<int>(props.limits.maxImageDimension2D / 1000);

        std::cout << "  GPU: " << props.deviceName
                  << " | Type: " << props.deviceType
                  << " | Score: " << score << std::endl;

        if (score > 0) {
            candidates.insert(std::make_pair(score, device));
        }
    }

    if (candidates.empty()) {
        MessageBoxA(g_hwnd,
            "No suitable Vulkan GPU found!\n"
            "Your GPU may not support required features.",
            "BrushEngine Error", MB_OK | MB_ICONERROR);
        return false;
    }

    g_physicalDevice = candidates.rbegin()->second;

    VkPhysicalDeviceProperties props;
    vkGetPhysicalDeviceProperties(g_physicalDevice, &props);
    std::cout << "Selected GPU: " << props.deviceName << std::endl;

    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(g_physicalDevice, &queueFamilyCount, nullptr);
    std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(g_physicalDevice, &queueFamilyCount, queueFamilies.data());

    std::optional<uint32_t> graphicsFamily;
    std::optional<uint32_t> presentFamily;

    for (uint32_t i = 0; i < queueFamilyCount; ++i) {
        if (queueFamilies[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
            if (!graphicsFamily.has_value()) graphicsFamily = i;
        }

        VkBool32 presentSupport = false;
        vkGetPhysicalDeviceSurfaceSupportKHR(g_physicalDevice, i, g_surface, &presentSupport);
        if (presentSupport) {
            if (!presentFamily.has_value()) presentFamily = i;
        }

        if ((queueFamilies[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) && presentSupport) {
            graphicsFamily = i;
            presentFamily = i;
            break;
        }
    }

    if (!graphicsFamily.has_value() || !presentFamily.has_value()) {
        MessageBoxA(g_hwnd, "GPU does not support required queue families.", "BrushEngine Error", MB_OK | MB_ICONERROR);
        return false;
    }

    g_graphicsQueueFamily = graphicsFamily.value();
    g_presentQueueFamily = presentFamily.value();

    return true;
}

bool createLogicalDevice() {
    std::set<uint32_t> uniqueQueueFamilies = { g_graphicsQueueFamily, g_presentQueueFamily };
    std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;

    float queuePriority = 1.0f;
    for (uint32_t queueFamily : uniqueQueueFamilies) {
        VkDeviceQueueCreateInfo queueCreateInfo = {};
        queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queueCreateInfo.queueFamilyIndex = queueFamily;
        queueCreateInfo.queueCount = 1;
        queueCreateInfo.pQueuePriorities = &queuePriority;
        queueCreateInfos.push_back(queueCreateInfo);
    }

    VkPhysicalDeviceFeatures deviceFeatures = {};

    const char* deviceExtensions[] = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };

    VkDeviceCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
    createInfo.pQueueCreateInfos = queueCreateInfos.data();
    createInfo.pEnabledFeatures = &deviceFeatures;
    createInfo.enabledExtensionCount = 1;
    createInfo.ppEnabledExtensionNames = deviceExtensions;

    VK_CHECK(vkCreateDevice(g_physicalDevice, &createInfo, nullptr, &g_device));

    vkGetDeviceQueue(g_device, g_graphicsQueueFamily, 0, &g_graphicsQueue);
    vkGetDeviceQueue(g_device, g_presentQueueFamily, 0, &g_presentQueue);
    return true;
}