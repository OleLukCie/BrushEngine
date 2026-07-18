#pragma once

#include "vk_composite_pipeline.h"
#include <vulkan/vulkan.h>
#include <vector>
#include <string>
#include <memory>

// ============================================================================
// GpuCanvas - GPU 画布管理
//
// 职责：
//   1. 管理 GPU 图层纹理（VkImage + VkImageView + VkDeviceMemory）
//   2. 维护图层属性（opacity, blendMode, visible, locked）
//   3. 提供图层合成入口（composite）
//   4. 跟踪激活图层（active layer）
//
// 不继承现有 Canvas 类，完全独立实现。
// 与 CPU Canvas 的交互通过显式的 import/export 方法进行。
// ============================================================================

class GpuCanvas {
public:
    // -------------------------------------------------------------------------
    // 生命周期
    // -------------------------------------------------------------------------
    GpuCanvas();
    ~GpuCanvas();

    // 禁止拷贝，允许移动
    GpuCanvas(const GpuCanvas&) = delete;
    GpuCanvas& operator=(const GpuCanvas&) = delete;
    GpuCanvas(GpuCanvas&& other) noexcept;
    GpuCanvas& operator=(GpuCanvas&& other) noexcept;

    // -------------------------------------------------------------------------
    // 初始化与清理
    // -------------------------------------------------------------------------

    // 初始化画布，创建背景图层
    // 参数:
    //   width  - 画布宽度（像素）
    //   height - 画布高度（像素）
    // 返回: 成功返回 true
    bool init(int width, int height);

    // 清理所有 GPU 资源
    void cleanup();

    // -------------------------------------------------------------------------
    // 基本属性
    // -------------------------------------------------------------------------
    int width() const { return width_; }
    int height() const { return height_; }

    // -------------------------------------------------------------------------
    // 图层管理
    // -------------------------------------------------------------------------

    // 获取图层数量
    size_t layerCount() const { return layers_.size(); }

    // 在末尾添加新图层，返回图层索引
    size_t addLayer(const std::string& name = "Layer");

    // 在指定位置插入图层
    size_t addLayer(size_t index, const std::string& name = "Layer");

    // 删除指定图层
    void removeLayer(size_t index);

    // 移动图层顺序
    void moveLayer(size_t fromIndex, size_t toIndex);

    // 复制图层
    void duplicateLayer(size_t index);

    // 向下合并图层
    void mergeDown(size_t index);

    // 合并所有可见图层到背景层
    void flatten();

    // -------------------------------------------------------------------------
    // 激活图层
    // -------------------------------------------------------------------------
    size_t activeLayerIndex() const { return activeLayerIndex_; }
    void setActiveLayer(size_t index);

    // -------------------------------------------------------------------------
    // 图层属性访问
    // -------------------------------------------------------------------------
    const std::string& layerName(size_t index) const;
    void setLayerName(size_t index, const std::string& name);

    float layerOpacity(size_t index) const;
    void setLayerOpacity(size_t index, float opacity);

    uint32_t layerBlendMode(size_t index) const;
    void setLayerBlendMode(size_t index, uint32_t mode);

    bool layerVisible(size_t index) const;
    void setLayerVisible(size_t index, bool visible);

    bool layerLocked(size_t index) const;
    void setLayerLocked(size_t index, bool locked);

    // -------------------------------------------------------------------------
    // GPU 资源访问（供笔刷渲染器使用）
    // -------------------------------------------------------------------------

    // 获取指定图层的图像视图（用于笔刷渲染目标）
    VkImageView layerImageView(size_t index) const;

    // 获取指定图层的图像句柄（用于 barrier）
    VkImage layerImage(size_t index) const;

    // 获取指定图层的 framebuffer（用于笔刷 render pass）
    VkFramebuffer layerFramebuffer(size_t index) const;

    // -------------------------------------------------------------------------
    // 合成
    // -------------------------------------------------------------------------

    // 将所有图层合成为最终图像
    // 参数:
    //   cmd        - 目标 command buffer（recording 状态）
    //   outputView - 输出图像视图（如 swapchain image view 或中间纹理）
    // 返回: 成功合成的图层数量
    uint32_t composite(VkCommandBuffer cmd, VkImageView outputView);

    // -------------------------------------------------------------------------
    // 数据导入导出
    // -------------------------------------------------------------------------

    // 从 CPU 像素数据导入到指定图层
    // 参数:
    //   index - 目标图层索引
    //   rgba8 - RGBA8 格式像素数据，大小必须为 width * height * 4
    void importLayerPixels(size_t index, const std::vector<uint8_t>& rgba8);

    // 导出指定图层像素到 CPU
    std::vector<uint8_t> exportLayerPixels(size_t index) const;

    // 获取画布背景色
    Color backgroundColor() const { return backgroundColor_; }
    void setBackgroundColor(const Color& color) { backgroundColor_ = color; }

private:
    // -------------------------------------------------------------------------
    // 内部结构
    // -------------------------------------------------------------------------
    struct GpuLayer {
        std::string name;
        VkImage image = VK_NULL_HANDLE;
        VkDeviceMemory memory = VK_NULL_HANDLE;
        VkImageView view = VK_NULL_HANDLE;
        VkFramebuffer framebuffer = VK_NULL_HANDLE;
        float opacity = 1.0f;
        uint32_t blendMode = 0;    // 0=Normal
        bool visible = true;
        bool locked = false;
    };

    // -------------------------------------------------------------------------
    // 成员变量
    // -------------------------------------------------------------------------
    int width_ = 0;
    int height_ = 0;
    std::vector<GpuLayer> layers_;
    size_t activeLayerIndex_ = 0;
    Color backgroundColor_ = Color(1.0f, 1.0f, 1.0f, 1.0f);  // 默认白色

    // 合成管线状态（来自 vk_composite_pipeline.h）
    CompositePipelineState compositeState_;
    bool compositeInitialized_ = false;

    // 图层纹理通用采样器
    VkSampler layerSampler_ = VK_NULL_HANDLE;

    // -------------------------------------------------------------------------
    // 内部方法
    // -------------------------------------------------------------------------

    // 创建单个 GPU 图层（图像 + 内存 + 视图 + framebuffer）
    bool createGpuLayer(GpuLayer& layer);

    // 销毁单个 GPU 图层
    void destroyGpuLayer(GpuLayer& layer);

    // 创建图层纹理通用采样器
    bool createLayerSampler();

    // 验证图层索引有效性
    bool isValidLayerIndex(size_t index) const {
        return index < layers_.size();
    }

    // 获取可见图层的 views 和 params，用于合成
    void gatherVisibleLayers(std::vector<VkImageView>& outViews,
                              std::vector<CompositeLayerParams>& outParams) const;
};
