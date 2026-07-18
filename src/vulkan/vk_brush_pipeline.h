#pragma once
#include <vulkan/vulkan.h>
#include "../base/base_types.h"
#include <vector>

// ============================================================================
// DabInstance - GPU 笔刷圆点实例数据
// 对应 CPU 侧的 BrushRenderState + BrushSettings 的 GPU 表示
//
// 内存布局严格遵循 Vulkan std430 规则（SSBO/PushConstant 默认布局）：
//   - vec4 (Color):  align 16, size 16
//   - vec2 (Vec2):   align 8,  size 8
//   - float:         align 4,  size 4
//   - uint32_t:      align 4,  size 4
//
// 字段按对齐要求从大到小排列，避免 implicit padding 导致 CPU/GPU 不匹配。
// 总大小: 64 bytes（含末尾 padding 到 16 字节对齐）
//
// GLSL 侧对应声明：
//   struct DabInstance {
//       vec4  color;
//       vec2  center;
//       float size;
//       float opacity;
//       float rotation;
//       float flattening;
//       float textureScale;
//       float textureRotation;
//       uint  useTexture;
//       uint  blendMode;
//   };
// ============================================================================
struct alignas(16) DabInstance {
    // --- 16-byte aligned block ---
    Color color = Color{ 0, 0, 0, 0 };              // offset 0,  size 16, align 16

    // --- 8-byte aligned block ---
    Vec2 center = Vec2{0.0f, 0.0f};              // offset 16, size 8,  align 8

    // --- 4-byte aligned scalars ---
    float size = 0.0f;               // offset 24, size 4,  align 4
    float opacity = 1.0f;            // offset 28, size 4,  align 4
    float rotation = 0.0f;           // offset 32, size 4,  align 4
    float flattening = 0.0f;         // offset 36, size 4,  align 4
    float textureScale = 1.0f;       // offset 40, size 4,  align 4
    float textureRotation = 0.0f;    // offset 44, size 4,  align 4
    uint32_t useTexture = 0;      // offset 48, size 4,  align 4
    uint32_t blendMode = 0;       // offset 52, size 4,  align 4

    // padding to 64 bytes (next multiple of 16)
    uint32_t _padding[2] = {0, 0};     // offset 56, size 8
};
static_assert(sizeof(DabInstance) == 64, "DabInstance must be exactly 64 bytes");
static_assert(alignof(DabInstance) >= 16, "DabInstance alignment too small");

// ============================================================================
// BrushPushConstants - 笔刷管线 Push Constants
// 每 stroke 调用更新一次，通过 vkCmdPushConstants 直接传入 command buffer
//
// 内存布局遵循 Vulkan std430 规则：
//   - vec2 (Vec2):  align 8, size 8
//   - float:        align 4, size 4
//   - uint32_t:     align 4, size 4
//
// 总大小: 16 bytes（含末尾 padding 到 8 字节对齐）
//
// GLSL 侧对应声明：
//   layout(push_constant) uniform PushConstants {
//       vec2  canvasSize;
//       float globalOpacity;
//       int   numDabs;
//   } pc;
// ============================================================================
struct alignas(16) BrushPushConstants {
    Vec2 canvasSize;          // offset 0,  size 8,  align 8
    float globalOpacity;      // offset 8,  size 4,  align 4
    uint32_t numDabs;         // offset 12, size 4,  align 4
};
static_assert(sizeof(BrushPushConstants) == 16, "BrushPushConstants must be exactly 16 bytes");
static_assert(alignof(BrushPushConstants) >= 4, "BrushPushConstants alignment too small");

// ============================================================================
// 笔刷管线全局状态
// 管理 brush_dab.vert + brush_dab.frag 的管线对象、描述符布局、资源
// ============================================================================
struct BrushPipelineState {
    VkPipeline pipeline = VK_NULL_HANDLE;           // 图形管线
    VkPipelineLayout layout = VK_NULL_HANDLE;       // 管线布局（含 push constant range + descriptor set layout）
    VkDescriptorSetLayout descSetLayout = VK_NULL_HANDLE; // 描述符集布局（SSBO + texture sampler）
    VkRenderPass renderPass = VK_NULL_HANDLE;       // 笔刷渲染专用的 render pass（单 attachment，无 depth）
};

// ============================================================================
// 笔刷纹理 GPU 表示
// 从 CPU 侧的 Texture 对象上传到 GPU 的 VkImage + VkImageView + VkSampler
// ============================================================================
struct BrushTexture {
    VkImage image = VK_NULL_HANDLE;
    VkDeviceMemory memory = VK_NULL_HANDLE;
    VkImageView view = VK_NULL_HANDLE;
    VkSampler sampler = VK_NULL_HANDLE;
    uint32_t width = 0;
    uint32_t height = 0;
};

// ============================================================================
// 创建/销毁笔刷管线
// 在 Vulkan 初始化阶段调用 create，清理阶段调用 destroy
// ============================================================================
bool createBrushPipeline(BrushPipelineState& outState, VkRenderPass sharedRenderPass);
void destroyBrushPipeline(BrushPipelineState& state);

// ============================================================================
// 记录笔刷渲染命令到 command buffer
//
// 参数:
//   cmd          - 目标 command buffer（必须在 recording 状态）
//   state        - 已创建的笔刷管线状态
//   dabs         - dab 实例数组（CPU 侧数据，函数内部上传到 GPU SSBO）
//   targetImage  - 渲染目标图像（图层纹理）
//   targetView   - 渲染目标图像视图（用于 framebuffer attachment）
//   targetWidth  - 目标图像宽度
//   targetHeight - 目标图像高度
//   brushTex     - 可选的笔刷纹理（nullptr = 纯色笔刷）
//
// 返回:
//   实际渲染的 dab 数量（0 表示失败或空数组）
//
// 注意:
//   - 此函数会创建临时 framebuffer 并绑定 render pass
//   - 调用方需要确保 targetImage 当前布局为 COLOR_ATTACHMENT_OPTIMAL
//   - SSBO 数据通过 staging buffer + vkCmdCopyBuffer 上传
// ============================================================================
uint32_t recordBrushRender(
    VkCommandBuffer cmd,
    const BrushPipelineState& state,
    const std::vector<DabInstance>& dabs,
    VkImage targetImage,
    VkImageView targetView,
    uint32_t targetWidth,
    uint32_t targetHeight,
    const BrushTexture* brushTex = nullptr);

// ============================================================================
// 从 CPU Texture 创建 GPU 笔刷纹理
//
// 参数:
//   texture - CPU 侧的 Texture 对象指针
//
// 返回:
//   成功创建的 BrushTexture，失败时所有字段为 VK_NULL_HANDLE
//
// 注意:
//   - 创建的 texture 需要调用方在不再使用时手动 destroyBrushTexture
//   - 内部使用 staging buffer 上传像素数据
// ============================================================================
BrushTexture createBrushTexture(const class Texture* texture);

// ============================================================================
// 销毁笔刷纹理
// ============================================================================
void destroyBrushTexture(BrushTexture& tex);