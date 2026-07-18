#define NOMINMAX
#include "base/log.h"
#include "vk_buffer.h"
#include "vk_utils.h"
#include <cstring>

extern VkDevice g_device;
extern VkPhysicalDevice g_physicalDevice;

// ============================================================================
// GpuBuffer
// ============================================================================

bool GpuBuffer::create(VkDeviceSize size, VkBufferUsageFlags usage,
                       VkMemoryPropertyFlags props) {
    this->size = size;

    // 1. Create buffer
    VkBufferCreateInfo bufferInfo = {};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = size;
    bufferInfo.usage = usage;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    VK_CHECK(vkCreateBuffer(g_device, &bufferInfo, nullptr, &buffer));
    if (buffer == VK_NULL_HANDLE) {
        return false;
    }

    // 2. Query memory requirements
    VkMemoryRequirements memReq;
    vkGetBufferMemoryRequirements(g_device, buffer, &memReq);

    // 3. Allocate memory
    VkMemoryAllocateInfo allocInfo = {};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memReq.size;
    allocInfo.memoryTypeIndex = findMemoryType(g_physicalDevice, memReq.memoryTypeBits, props);

    VK_CHECK(vkAllocateMemory(g_device, &allocInfo, nullptr, &memory));
    if (memory == VK_NULL_HANDLE) {
        vkDestroyBuffer(g_device, buffer, nullptr);
        buffer = VK_NULL_HANDLE;
        return false;
    }

    // 4. Bind memory to buffer
    VK_CHECK(vkBindBufferMemory(g_device, buffer, memory, 0));

    // 5. If host visible, persistently map
    if (props & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT) {
        VK_CHECK(vkMapMemory(g_device, memory, 0, size, 0, &mapped));
        if (mapped == nullptr) {
            vkFreeMemory(g_device, memory, nullptr);
            vkDestroyBuffer(g_device, buffer, nullptr);
            memory = VK_NULL_HANDLE;
            buffer = VK_NULL_HANDLE;
            return false;
        }
    }

    return true;
}

void GpuBuffer::destroy() {
    if (mapped != nullptr) {
        vkUnmapMemory(g_device, memory);
        mapped = nullptr;
    }
    if (memory != VK_NULL_HANDLE) {
        vkFreeMemory(g_device, memory, nullptr);
        memory = VK_NULL_HANDLE;
    }
    if (buffer != VK_NULL_HANDLE) {
        vkDestroyBuffer(g_device, buffer, nullptr);
        buffer = VK_NULL_HANDLE;
    }
    size = 0;
}

bool GpuBuffer::upload(const void* data, VkDeviceSize offset, VkDeviceSize dataSize) {
    if (data == nullptr || dataSize == 0) {
        return true;
    }
    if (offset + dataSize > size) {
        BE_LOG_ERROR(
            "GpuBuffer::upload overflow: offset={}, dataSize={}, bufferSize={}",
            offset,
            dataSize,
            size
        );
        return false;
    }

    if (mapped != nullptr) {
        // Host visible: direct memcpy
        std::memcpy(static_cast<char*>(mapped) + offset, data, dataSize);

        // If not coherent, flush
        // We need to check actual memory properties to decide if flush is needed.
        // For simplicity in this codebase, we usually allocate with HOST_COHERENT_BIT
        // alongside HOST_VISIBLE_BIT. If only HOST_VISIBLE_BIT is set (without coherent),
        // we should call vkFlushMappedMemoryRanges.
        //
        // To properly handle this, we would need to store the property flags.
        // For now, we assume coherent or let the caller ensure proper synchronization.
    } else {
        // Device local: need staging buffer + command buffer copy
        // This function does not have access to command buffers,
        // so device-local upload must be done externally via vkCmdCopyBuffer.
        // We create a temporary staging buffer here but cannot perform the copy.
        BE_LOG_ERROR("GpuBuffer::upload: device-local buffer upload requires external command buffer copy");
        return false;
    }

    return true;
}

// ============================================================================
// RingBuffer
// ============================================================================

bool RingBuffer::init(VkDeviceSize perFrameSize, uint32_t frames,
                      VkBufferUsageFlags usage) {
    this->frameSize = perFrameSize;
    this->frameCount = frames;
    this->frameIndex = 0;

    VkDeviceSize totalSize = perFrameSize * frames;

    // Ring buffer is always host visible for frequent CPU writes
    VkMemoryPropertyFlags props = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                                   VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;

    if (!gpuBuffer.create(totalSize, usage, props)) {
        return false;
    }

    return true;
}

void* RingBuffer::mapCurrentFrame() {
    if (gpuBuffer.mapped == nullptr) {
        return nullptr;
    }
    return static_cast<char*>(gpuBuffer.mapped) + currentOffset();
}

VkDeviceSize RingBuffer::currentOffset() const {
    return static_cast<VkDeviceSize>(frameIndex) * frameSize;
}

void RingBuffer::nextFrame() {
    frameIndex = (frameIndex + 1) % frameCount;
}