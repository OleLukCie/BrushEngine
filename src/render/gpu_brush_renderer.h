#pragma once

#include "vk_brush_pipeline.h"
#include "brush_renderer.h"  // 仅用于 BrushSettings, BrushRenderState, Vec2, Color
#include <vulkan/vulkan.h>
#include <vector>

// ============================================================================
// GpuBrushRenderer - GPU 笔刷渲染器
//
// 职责：
//   1. 接收笔刷绘制指令（stroke, dab），缓冲到 CPU 侧
//   2. 在 flush 时批量上传到 GPU 并渲染
//   3. 管理笔刷纹理（从 CPU Texture 上传到 GPU）
//   4. 提供与 BrushRenderer 类似的接口，但底层走 GPU
//
// 不继承 BrushRenderer，完全独立实现。
// 原因：
//   - BrushRenderer 依赖 CPURenderTarget，GPU 版本不需要
//   - GPU 版本的渲染是异步/批量的，与 CPU 版本的同步逐像素不同
//   - 参数和返回值语义不同（GPU 版本缓冲 dab，flush 时统一渲染）
//
// 使用方式：
//   GpuBrushRenderer renderer;
//   renderer.init();
//   renderer.setSettings(mySettings);
//   renderer.setTarget(layerImage, layerView, width, height);
//   renderer.setBrushTexture(brushTex);
//
//   // 绘制过程中不断添加 stroke
//   renderer.renderStroke(from, to, fromState, toState);
//   renderer.renderStrokePath(states);
//
//   // 每帧结束时统一提交
//   renderer.flush(cmd);
// ============================================================================

class GpuBrushRenderer {
public:
    // -------------------------------------------------------------------------
    // 生命周期
    // -------------------------------------------------------------------------
    GpuBrushRenderer();
    ~GpuBrushRenderer();

    // 禁止拷贝，允许移动
    GpuBrushRenderer(const GpuBrushRenderer&) = delete;
    GpuBrushRenderer& operator=(const GpuBrushRenderer&) = delete;
    GpuBrushRenderer(GpuBrushRenderer&& other) noexcept;
    GpuBrushRenderer& operator=(GpuBrushRenderer&& other) noexcept;

    // -------------------------------------------------------------------------
    // 初始化与清理
    // -------------------------------------------------------------------------

    // 初始化 GPU 管线资源
    // 使用全局 Vulkan 设备（g_device, g_physicalDevice）
    // 返回: 成功返回 true
    bool init();

    // 清理所有 GPU 资源
    void cleanup();

    // -------------------------------------------------------------------------
    // 配置
    // -------------------------------------------------------------------------

    // 设置笔刷配置（与 BrushSettings 兼容）
    void setSettings(const BrushSettings& settings) { settings_ = settings; }

    // 获取当前笔刷配置
    const BrushSettings& settings() const { return settings_; }
    BrushSettings& settings() { return settings_; }

    // -------------------------------------------------------------------------
    // 渲染目标
    // -------------------------------------------------------------------------

    // 设置渲染目标图层
    // 参数:
    //   image  - 目标图层 VkImage（用于 barrier）
    //   view   - 目标图层 VkImageView（用于 framebuffer attachment）
    //   width  - 目标宽度
    //   height - 目标高度
    void setTarget(VkImage image, VkImageView view, uint32_t width, uint32_t height);

    // 清除当前渲染目标
    void clearTarget();

    // -------------------------------------------------------------------------
    // 笔刷纹理
    // -------------------------------------------------------------------------

    // 设置笔刷纹理（从 CPU Texture 上传）
    // 参数:
    //   texture - CPU 侧 Texture 对象指针
    // 注意: 内部会创建/更新 GPU 纹理，之前的笔刷纹理会被销毁
    void setBrushTexture(const class Texture* texture);

    // 清除笔刷纹理（恢复为纯色椭圆笔刷）
    void clearBrushTexture();

    // -------------------------------------------------------------------------
    // 渲染接口
    // -------------------------------------------------------------------------

    // 渲染两点之间的 stroke
    // CPU 侧计算插值 dab，缓冲到 pendingDabs_，不立即提交 GPU
    // 返回: 本次 stroke 产生的 dab 数量（0 表示失败或距离过短）
    int renderStroke(const Vec2& from, const Vec2& to,
                     const BrushRenderState& fromState,
                     const BrushRenderState& toState);

    // 渲染多点路径 stroke
    // 返回: 本次 path 产生的 dab 总数
    int renderStrokePath(const std::vector<BrushRenderState>& states);

    // 渲染平滑曲线 stroke
    // 返回: 本次 stroke 产生的 dab 总数
    int renderSmoothStroke(const std::vector<Vec2>& points,
                           const std::vector<float>& pressures,
                           float smoothing = 0.5f);

    // -------------------------------------------------------------------------
    // 提交
    // -------------------------------------------------------------------------

    // 将所有缓冲的 dab 批量提交到 GPU 渲染
    // 应在每帧渲染循环中调用一次
    // 参数:
    //   cmd - 目标 command buffer（recording 状态）
    // 返回: 实际渲染的 dab 数量
    uint32_t flush(VkCommandBuffer cmd);

    // 检查是否有未提交的 dab
    bool hasPendingDabs() const { return !pendingDabs_.empty(); }

    // 清除未提交的 dab（不渲染）
    void clearPendingDabs();

private:
    // -------------------------------------------------------------------------
    // 成员变量
    // -------------------------------------------------------------------------

    // 笔刷配置
    BrushSettings settings_;

    // GPU 管线状态
    BrushPipelineState pipelineState_;
    bool pipelineInitialized_ = false;

    // 当前渲染目标
    VkImage targetImage_ = VK_NULL_HANDLE;
    VkImageView targetView_ = VK_NULL_HANDLE;
    uint32_t targetWidth_ = 0;
    uint32_t targetHeight_ = 0;

    // 笔刷纹理
    BrushTexture brushTexture_;
    bool hasBrushTexture_ = false;

    // 待渲染的 dab 缓冲区（CPU 侧）
    std::vector<DabInstance> pendingDabs_;

    // 每帧最大 dab 数量限制
    static constexpr uint32_t MAX_PENDING_DABS = 4096;

    // -------------------------------------------------------------------------
    // 内部方法
    // -------------------------------------------------------------------------

    // 将 BrushRenderState + BrushSettings 转换为 DabInstance
    DabInstance buildDabInstance(const BrushRenderState& state) const;

    // 计算 stroke 的 dab 数量（对应 BrushRenderer::renderStroke 的插值逻辑）
    int computeStrokeDabs(const Vec2& from, const Vec2& to,
                          float fromPressure, float toPressure) const;

    // 提交 pendingDabs_ 到 GPU（被 flush 调用）
    uint32_t submitDabs(VkCommandBuffer cmd);

    // 辅助：计算笔刷大小
    float computeSize(float pressure) const;

    // 辅助：计算透明度
    float computeOpacity(const BrushRenderState& state, float strokeT) const;

    // 辅助：计算旋转角度
    float computeRotation(const Vec2& direction) const;
};
