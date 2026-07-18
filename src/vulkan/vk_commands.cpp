#define NOMINMAX
#include "base/log.h"
#include "vk_commands.h"
#include "vk_utils.h"
#include "../engine/engine_state.h"

bool createCommandPool() {
    VkCommandPoolCreateInfo poolInfo = {};
    poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolInfo.queueFamilyIndex = g_graphicsQueueFamily;
    poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

    VK_CHECK(vkCreateCommandPool(g_device, &poolInfo, nullptr, &g_commandPool));
    return true;
}

bool createCommandBuffers() {
    g_commandBuffers.resize(g_maxFramesInFlight);

    VkCommandBufferAllocateInfo allocInfo = {};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = g_commandPool;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = static_cast<uint32_t>(g_commandBuffers.size());

    VK_CHECK(vkAllocateCommandBuffers(g_device, &allocInfo, g_commandBuffers.data()));
    return true;
}