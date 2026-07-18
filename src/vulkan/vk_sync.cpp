#define NOMINMAX
#include "base/log.h"
#include "vk_sync.h"
#include "vk_utils.h"
#include "../engine/engine_state.h"

bool createSyncObjects() {
    g_imageAvailableSemaphores.resize(g_maxFramesInFlight);
    g_renderFinishedSemaphores.resize(g_maxFramesInFlight);
    g_inFlightFences.resize(g_maxFramesInFlight);

    VkSemaphoreCreateInfo semaphoreInfo = {};
    semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    VkFenceCreateInfo fenceInfo = {};
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    for (uint32_t i = 0; i < g_maxFramesInFlight; ++i) {
        VK_CHECK(vkCreateSemaphore(g_device, &semaphoreInfo, nullptr, &g_imageAvailableSemaphores[i]));
        VK_CHECK(vkCreateSemaphore(g_device, &semaphoreInfo, nullptr, &g_renderFinishedSemaphores[i]));
        VK_CHECK(vkCreateFence(g_device, &fenceInfo, nullptr, &g_inFlightFences[i]));
    }

    g_imagesInFlight.resize(g_imageCount, UINT32_MAX);  // UINT32_MAX 表示未被占用
    
    return true;
}