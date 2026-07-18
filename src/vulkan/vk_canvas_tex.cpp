#define NOMINMAX
#include "base/log.h"
#include "vk_canvas_tex.h"
#include "vk_utils.h"
#include "../engine/engine_state.h"

#include "render/canvas.h"

#include <imgui.h>
#include <imgui_impl_vulkan.h>

bool createCanvasTexture(int width, int height) {
    g_canvasStagingSize = width * height * 4;

    // 1. Staging Buffer (host visible)
    VkBufferCreateInfo bufferInfo = {};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = g_canvasStagingSize;
    bufferInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
    if (vkCreateBuffer(g_device, &bufferInfo, nullptr, &g_canvasStagingBuffer) != VK_SUCCESS)
        return false;

    VkMemoryRequirements stagingReq;
    vkGetBufferMemoryRequirements(g_device, g_canvasStagingBuffer, &stagingReq);
    VkMemoryAllocateInfo stagingAlloc = {};
    stagingAlloc.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    stagingAlloc.allocationSize = stagingReq.size;
    stagingAlloc.memoryTypeIndex = findMemoryType(g_physicalDevice, stagingReq.memoryTypeBits,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
    if (vkAllocateMemory(g_device, &stagingAlloc, nullptr, &g_canvasStagingBufferMemory) != VK_SUCCESS)
        return false;
    if (vkBindBufferMemory(g_device, g_canvasStagingBuffer, g_canvasStagingBufferMemory, 0) != VK_SUCCESS)
        return false;

    // 2. Device Local Image
    VkImageCreateInfo imageInfo = {};
    imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageInfo.imageType = VK_IMAGE_TYPE_2D;
    imageInfo.extent.width = width;
    imageInfo.extent.height = height;
    imageInfo.extent.depth = 1;
    imageInfo.mipLevels = 1;
    imageInfo.arrayLayers = 1;
    imageInfo.format = VK_FORMAT_R8G8B8A8_UNORM;
    imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
    imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    imageInfo.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
    imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
    if (vkCreateImage(g_device, &imageInfo, nullptr, &g_canvasImage) != VK_SUCCESS)
        return false;

    VkMemoryRequirements imgReq;
    vkGetImageMemoryRequirements(g_device, g_canvasImage, &imgReq);
    VkMemoryAllocateInfo imgAlloc = {};
    imgAlloc.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    imgAlloc.allocationSize = imgReq.size;
    imgAlloc.memoryTypeIndex = findMemoryType(g_physicalDevice, imgReq.memoryTypeBits,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    if (vkAllocateMemory(g_device, &imgAlloc, nullptr, &g_canvasImageMemory) != VK_SUCCESS)
        return false;
    if (vkBindImageMemory(g_device, g_canvasImage, g_canvasImageMemory, 0) != VK_SUCCESS)
        return false;

    // 3. Image View
    VkImageViewCreateInfo viewInfo = {};
    viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    viewInfo.image = g_canvasImage;
    viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    viewInfo.format = VK_FORMAT_R8G8B8A8_UNORM;
    viewInfo.subresourceRange = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 };
    if (vkCreateImageView(g_device, &viewInfo, nullptr, &g_canvasImageView) != VK_SUCCESS)
        return false;

    // 4. Sampler
    VkSamplerCreateInfo samplerInfo = {};
    samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    samplerInfo.magFilter = VK_FILTER_LINEAR;
    samplerInfo.minFilter = VK_FILTER_LINEAR;
    samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    samplerInfo.anisotropyEnable = VK_FALSE;
    samplerInfo.maxAnisotropy = 1.0f;
    samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
    samplerInfo.unnormalizedCoordinates = VK_FALSE;
    samplerInfo.compareEnable = VK_FALSE;
    samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    if (vkCreateSampler(g_device, &samplerInfo, nullptr, &g_canvasSampler) != VK_SUCCESS)
        return false;

    // 5. 注册到 ImGui
    g_canvasTextureId = (ImTextureID)ImGui_ImplVulkan_AddTexture(g_canvasSampler, g_canvasImageView, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

    g_canvasFirstUpload = true;
    return true;
}

void destroyCanvasTexture() {
    if (g_canvasTextureId != (ImTextureID)0) {
        ImGui_ImplVulkan_RemoveTexture((VkDescriptorSet)g_canvasTextureId);
        g_canvasTextureId = (ImTextureID)0;
    }

    if (g_canvasSampler != VK_NULL_HANDLE) { vkDestroySampler(g_device, g_canvasSampler, nullptr); g_canvasSampler = VK_NULL_HANDLE; }
    if (g_canvasImageView != VK_NULL_HANDLE) { vkDestroyImageView(g_device, g_canvasImageView, nullptr); g_canvasImageView = VK_NULL_HANDLE; }
    if (g_canvasImage != VK_NULL_HANDLE) { vkDestroyImage(g_device, g_canvasImage, nullptr); g_canvasImage = VK_NULL_HANDLE; }
    if (g_canvasImageMemory != VK_NULL_HANDLE) { vkFreeMemory(g_device, g_canvasImageMemory, nullptr); g_canvasImageMemory = VK_NULL_HANDLE; }
    if (g_canvasStagingBuffer != VK_NULL_HANDLE) { vkDestroyBuffer(g_device, g_canvasStagingBuffer, nullptr); g_canvasStagingBuffer = VK_NULL_HANDLE; }
    if (g_canvasStagingBufferMemory != VK_NULL_HANDLE) { vkFreeMemory(g_device, g_canvasStagingBufferMemory, nullptr); g_canvasStagingBufferMemory = VK_NULL_HANDLE; }
}

void updateCanvasTexture(VkCommandBuffer cmd, const std::vector<uint8_t>& rgba8Data) {
    if (rgba8Data.empty() || !g_canvasStagingBuffer) return;

    // 1. Map & copy to staging buffer
    void* data;
    vkMapMemory(g_device, g_canvasStagingBufferMemory, 0, g_canvasStagingSize, 0, &data);
    memcpy(data, rgba8Data.data(), rgba8Data.size());
    vkUnmapMemory(g_device, g_canvasStagingBufferMemory);

    // 2. Barrier: 首次 UNDEFINED -> TRANSFER_DST，后续 SHADER_READ_ONLY_OPTIMAL -> TRANSFER_DST
    VkImageLayout oldLayout = g_canvasFirstUpload ? VK_IMAGE_LAYOUT_UNDEFINED : VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    VkAccessFlags srcAccess = g_canvasFirstUpload ? 0 : VK_ACCESS_SHADER_READ_BIT;
    VkPipelineStageFlags srcStage = g_canvasFirstUpload ? VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT : VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;

    VkImageMemoryBarrier barrier1 = {};
    barrier1.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier1.oldLayout = oldLayout;
    barrier1.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    barrier1.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier1.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier1.image = g_canvasImage;
    barrier1.subresourceRange = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 };
    barrier1.srcAccessMask = srcAccess;
    barrier1.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

    vkCmdPipelineBarrier(
        cmd,
        srcStage,
        VK_PIPELINE_STAGE_TRANSFER_BIT,
        0, 0, nullptr, 0, nullptr, 1, &barrier1
    );

    // 3. Copy buffer to image
    VkBufferImageCopy region = {};
    region.bufferOffset = 0;
    region.bufferRowLength = 0;
    region.bufferImageHeight = 0;
    region.imageSubresource = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1 };
    region.imageExtent = { (uint32_t)g_canvas->width(), (uint32_t)g_canvas->height(), 1 };

    vkCmdCopyBufferToImage(cmd, g_canvasStagingBuffer, g_canvasImage,
        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);

    // 4. Barrier: TRANSFER_DST -> SHADER_READ_ONLY_OPTIMAL
    VkImageMemoryBarrier barrier2 = {};
    barrier2.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier2.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    barrier2.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    barrier2.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier2.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier2.image = g_canvasImage;
    barrier2.subresourceRange = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 };
    barrier2.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    barrier2.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

    vkCmdPipelineBarrier(cmd,
        VK_PIPELINE_STAGE_TRANSFER_BIT,
        VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
        0, 0, nullptr, 0, nullptr, 1, &barrier2);

    g_canvasFirstUpload = false;
}