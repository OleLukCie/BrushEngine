#pragma once
#include <vulkan/vulkan.h>
#include <vector>

struct GpuBuffer {
	VkBuffer buffer = VK_NULL_HANDLE;
	VkDeviceMemory memory = VK_NULL_HANDLE;
	VkDeviceSize size = 0;
	void* mapped = nullptr;

	bool create(VkDeviceSize size, VkBufferUsageFlags usage,
		VkMemoryPropertyFlags props);

	void destroy();

	bool upload(const void* data, VkDeviceSize offset, VkDeviceSize dataSize);
};

struct RingBuffer {
	GpuBuffer gpuBuffer;
	VkDeviceSize frameSize = 0;
	uint32_t frameIndex = 0;
	uint32_t frameCount = 0;

	bool init(VkDeviceSize perFrameSize, uint32_t frames,
		VkBufferUsageFlags usage);

	void* mapCurrentFrame();

	VkDeviceSize currentOffset() const;

	void nextFrame();
};