#pragma once
#include <vulkan/vulkan.h>
#include "../base/base_types.h"
#include <vector>

// ============================================================================
// CompositeLayerParams - 单图层合成参数
// 对应 CPU 侧 Layer::opacity() 和 Layer::blendMode()
//
// 内存布局：std140 友好（vec4 对齐），总大小 16 bytes
// GLSL 侧：vec4 layerParams; // x=opacity, y=blendMode, zw=reserved
// ============================================================================
struct CompositeLayerParams {
	float opacity;      // 图层不透明度 [0, 1]
	uint32_t blendMode; // 混合模式索引：0=Normal, 1=Multiply, 2=Screen, 3=Overlay, ...
	float _padding[2];  // 填充到 16 bytes
};
static_assert(sizeof(CompositeLayerParams) == 16, "CompositeLayerParams must be 16 bytes");

// ============================================================================
// CompositeUBO - 合成统一缓冲区
// 每帧更新一次，包含所有图层的合成参数
//
// GLSL 侧对应声明：
//   layout(set = 0, binding = 0) uniform CompositeUBO {
//       int   layerCount;
//       vec4  backgroundColor;
//       vec4  layerParams[16];
//   } ubo;
// ============================================================================
struct alignas(16) CompositeUBO {
	int32_t layerCount;                    // 实际图层数量
	float _padding1[3];                   // vec4 对齐填充
	Color backgroundColor;                // 画布背景色 RGBA
	CompositeLayerParams layerParams[16]; // 最多 16 个图层的参数
};
static_assert(sizeof(CompositeUBO) == 16 + 16 + 16 * 16, "CompositeUBO size mismatch");
static_assert(alignof(CompositeUBO) == 16, "CompositeUBO must be 16-byte aligned");

// ============================================================================
// CompositePipelineState - 合成管线全局状态
// ============================================================================
struct CompositePipelineState {
	VkPipeline pipeline = VK_NULL_HANDLE;           // 图形管线
	VkPipelineLayout layout = VK_NULL_HANDLE;       // 管线布局
	VkDescriptorSetLayout descSetLayout = VK_NULL_HANDLE; // 描述符集布局
	VkRenderPass renderPass = VK_NULL_HANDLE;       // 合成渲染通道
};

// ============================================================================
// 创建/销毁合成管线
// ============================================================================
bool createCompositePipeline(CompositePipelineState& outState, VkRenderPass sharedRenderPass);
void destroyCompositePipeline(CompositePipelineState& state);

// ============================================================================
// 记录合成命令到 command buffer
//
// 参数:
//   cmd          - 目标 command buffer（recording 状态）
//   state        - 已创建的合成管线状态
//   layerViews   - 图层纹理视图数组（顺序：0=底层 → N-1=顶层）
//   layerParams  - 每图层的合成参数（与 layerViews 一一对应）
//   background   - 画布背景色
//   outputView   - 输出目标图像视图（通常是 swapchain image 或中间纹理）
//   outputWidth  - 输出宽度
//   outputHeight - 输出高度
//
// 返回:
//   成功合成的图层数量（0 表示失败）
//
// 注意:
//   - layerViews.size() 必须 <= 16（MAX_LAYERS）
//   - 不可见图层的 texture view 仍需有效（可绑定 1x1 透明纹理）
//   - 调用方需确保 outputImage 当前布局为 COLOR_ATTACHMENT_OPTIMAL
// ============================================================================
uint32_t recordCompositeRender(
	VkCommandBuffer cmd,
	const CompositePipelineState& state,
	const std::vector<VkImageView>& layerViews,
	const std::vector<CompositeLayerParams>& layerParams,
	const Color& background,
	VkImageView outputView,
	uint32_t outputWidth,
	uint32_t outputHeight);
