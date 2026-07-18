#pragma once
#include <vulkan/vulkan.h>
#include <vector>
#include <cstdint>

bool createCanvasTexture(int width, int height);
void destroyCanvasTexture();
void updateCanvasTexture(VkCommandBuffer cmd, const std::vector<uint8_t>& rgba8Data);