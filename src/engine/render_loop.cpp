#define NOMINMAX
#include "render_loop.h"
#include "engine_state.h"
#include "input_sample.h"

#include "render/canvas.h"
#include "render/brush_renderer.h"
#include "sys/ui_manager.h"

#include "vulkan/vk_canvas_tex.h"
#include "vulkan/vk_swapchain.h"

#include <imgui.h>
#include <imgui_impl_vulkan.h>

#include <base/log.h>

void renderFrame() {
    BE_LOG_DEBUG("renderFrame start, currentFrame={}", g_currentFrame);

    // ===== 1. 等待当前 frame-in-flight 的 fence =====
    vkWaitForFences(g_device, 1, &g_inFlightFences[g_currentFrame], VK_TRUE, UINT64_MAX);
    vkResetFences(g_device, 1, &g_inFlightFences[g_currentFrame]);

    // ===== 2. 获取下一帧 image =====
    uint32_t imageIndex;
    VkResult result = vkAcquireNextImageKHR(g_device, g_swapchain, UINT64_MAX,
        g_imageAvailableSemaphores[g_currentFrame], VK_NULL_HANDLE, &imageIndex);

    BE_LOG_DEBUG("acquired image {}", imageIndex);

    if (result == VK_ERROR_OUT_OF_DATE_KHR) {
        recreateSwapchain();
        return;
    }
    else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
        return;
    }

    // ===== 3. 检查 image 是否被占用 =====
    if (g_imagesInFlight[imageIndex] != UINT32_MAX) {
        vkWaitForFences(g_device, 1, &g_inFlightFences[g_imagesInFlight[imageIndex]], VK_TRUE, UINT64_MAX);
    }
    g_imagesInFlight[imageIndex] = g_currentFrame;

    // ===== 4. CPU 端：准备 ImGui 帧（必须在 command buffer 录制前）=====
    BE_LOG_DEBUG("calling newFrame");
    g_uiManager->newFrame();

    // ===== 5. 录制命令 =====
    BE_LOG_DEBUG("begin command buffer");
    VkCommandBuffer cmd = g_commandBuffers[g_currentFrame];
    vkResetCommandBuffer(cmd, 0);

    VkCommandBufferBeginInfo beginInfo = {};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    vkBeginCommandBuffer(cmd, &beginInfo);

    // 上传画布纹理
    if (g_canvas) {
        std::vector<uint8_t> rgba8Data;
        g_canvas->exportToRGBA8(rgba8Data);
        updateCanvasTexture(cmd, rgba8Data);
    }

    // Render Pass
    VkRenderPassBeginInfo renderPassInfo = {};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass = g_renderPass;
    renderPassInfo.framebuffer = g_framebuffers[imageIndex];
    renderPassInfo.renderArea.offset = { 0, 0 };
    renderPassInfo.renderArea.extent = g_swapchainExtent;

    VkClearValue clearColor = { {{0.1f, 0.1f, 0.1f, 1.0f}} };
    renderPassInfo.clearValueCount = 1;
    renderPassInfo.pClearValues = &clearColor;

    BE_LOG_DEBUG("begin render pass");
    vkCmdBeginRenderPass(cmd, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

    // ===== 6. 在 render pass 内绘制 ImGui =====
    BE_LOG_DEBUG("calling render");
    g_uiManager->render(cmd);

    BE_LOG_DEBUG("end render pass");
    vkCmdEndRenderPass(cmd);
    vkEndCommandBuffer(cmd);

    // ===== 7. 提交 =====
    VkSemaphore waitSemaphores[] = { g_imageAvailableSemaphores[g_currentFrame] };
    VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
    VkSemaphore signalSemaphores[] = { g_renderFinishedSemaphores[g_currentFrame] };

    VkSubmitInfo submitInfo = {};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = waitSemaphores;
    submitInfo.pWaitDstStageMask = waitStages;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &cmd;
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = signalSemaphores;

    BE_LOG_DEBUG("submit");
    vkQueueSubmit(g_graphicsQueue, 1, &submitInfo, g_inFlightFences[g_currentFrame]);

    // ===== 8. Present =====
    VkSwapchainKHR swapChains[] = { g_swapchain };
    VkPresentInfoKHR presentInfo = {};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = signalSemaphores;
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = swapChains;
    presentInfo.pImageIndices = &imageIndex;

    BE_LOG_DEBUG("present");
    result = vkQueuePresentKHR(g_presentQueue, &presentInfo);

    if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || g_framebufferResized) {
        g_framebufferResized = false;
        recreateSwapchain();
        return;
    }

    // ===== 9. 推进 frame-in-flight =====
    BE_LOG_DEBUG("renderFrame end");
    g_currentFrame = (g_currentFrame + 1) % g_maxFramesInFlight;
}