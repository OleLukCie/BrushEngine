#define NOMINMAX
#include "gpu_canvas.h"
#include "vk_utils.h"
#include "vk_buffer.h"
#include <algorithm>
#include <cstring>
#include "base/log.h"

// ============================================================================
// 外部全局变量
// ============================================================================
extern VkDevice g_device;
extern VkPhysicalDevice g_physicalDevice;

// ============================================================================
// 内部常量
// ============================================================================
static constexpr VkFormat LAYER_FORMAT = VK_FORMAT_R8G8B8A8_UNORM;

// ============================================================================
// 生命周期
// ============================================================================

GpuCanvas::GpuCanvas() = default;

GpuCanvas::~GpuCanvas() {
    cleanup();
}

GpuCanvas::GpuCanvas(GpuCanvas&& other) noexcept {
    *this = std::move(other);
}

GpuCanvas& GpuCanvas::operator=(GpuCanvas&& other) noexcept {
    if (this != &other) {
        cleanup();
        width_ = other.width_;
        height_ = other.height_;
        layers_ = std::move(other.layers_);
        activeLayerIndex_ = other.activeLayerIndex_;
        backgroundColor_ = other.backgroundColor_;
        compositeState_ = other.compositeState_;
        compositeInitialized_ = other.compositeInitialized_;
        layerSampler_ = other.layerSampler_;

        other.width_ = 0;
        other.height_ = 0;
        other.activeLayerIndex_ = 0;
        other.compositeState_ = {};
        other.compositeInitialized_ = false;
        other.layerSampler_ = VK_NULL_HANDLE;
    }
    return *this;
}

// ============================================================================
// 初始化与清理
// ============================================================================

bool GpuCanvas::init(int width, int height) {
    if (width <= 0 || height <= 0) {
        BE_LOG_ERROR("GpuCanvas::init: invalid dimensions {}x{}", width, height);
        return false;
    }

    cleanup();

    width_ = width;
    height_ = height;

    // 创建合成管线
    if (!createCompositePipeline(compositeState_, VK_NULL_HANDLE)) {
        BE_LOG_ERROR("GpuCanvas::init: failed to create composite pipeline");
        cleanup();
        return false;
    }
    compositeInitialized_ = true;

    // 创建图层采样器
    if (!createLayerSampler()) {
        BE_LOG_ERROR("GpuCanvas::init: failed to create layer sampler");
        cleanup();
        return false;
    }

    // 创建默认背景图层
    if (addLayer("Background") == 0) {
        // 成功，背景层索引为 0
    } else {
        BE_LOG_ERROR("GpuCanvas::init: failed to create background layer");
        cleanup();
        return false;
    }

    return true;
}

void GpuCanvas::cleanup() {
    for (auto& layer : layers_) {
        destroyGpuLayer(layer);
    }
    layers_.clear();

    if (compositeInitialized_) {
        destroyCompositePipeline(compositeState_);
        compositeInitialized_ = false;
    }

    if (layerSampler_ != VK_NULL_HANDLE) {
        vkDestroySampler(g_device, layerSampler_, nullptr);
        layerSampler_ = VK_NULL_HANDLE;
    }

    width_ = 0;
    height_ = 0;
    activeLayerIndex_ = 0;
}

// ============================================================================
// 图层管理
// ============================================================================

size_t GpuCanvas::addLayer(const std::string& name) {
    return addLayer(layers_.size(), name);
}

size_t GpuCanvas::addLayer(size_t index, const std::string& name) {
    if (index > layers_.size()) {
        index = layers_.size();
    }

    GpuLayer layer;
    layer.name = name;

    if (!createGpuLayer(layer)) {
        BE_LOG_ERROR("GpuCanvas::addLayer: failed to create GPU layer");
        return layers_.size();
    }

    layers_.insert(layers_.begin() + index, std::move(layer));

    // 调整激活图层索引
    if (index <= activeLayerIndex_) {
        activeLayerIndex_++;
    }

    return index;
}

void GpuCanvas::removeLayer(size_t index) {
    if (!isValidLayerIndex(index)) {
        BE_LOG_ERROR("GpuCanvas::removeLayer: invalid index {}", index);
        return;
    }

    // 至少保留一个图层
    if (layers_.size() <= 1) {
        BE_LOG_ERROR("GpuCanvas::removeLayer: cannot remove the last layer");
        return;
    }

    destroyGpuLayer(layers_[index]);
    layers_.erase(layers_.begin() + index);

    // 调整激活图层索引
    if (activeLayerIndex_ >= layers_.size()) {
        activeLayerIndex_ = layers_.size() - 1;
    }
    if (activeLayerIndex_ >= index && activeLayerIndex_ > 0) {
        activeLayerIndex_--;
    }
}

void GpuCanvas::moveLayer(size_t fromIndex, size_t toIndex) {
    if (!isValidLayerIndex(fromIndex) || toIndex > layers_.size()) {
        return;
    }

    if (fromIndex == toIndex) return;

    auto layer = std::move(layers_[fromIndex]);
    layers_.erase(layers_.begin() + fromIndex);

    if (toIndex > fromIndex) {
        toIndex--;
    }
    layers_.insert(layers_.begin() + toIndex, std::move(layer));

    // 调整激活图层索引
    if (activeLayerIndex_ == fromIndex) {
        activeLayerIndex_ = toIndex;
    } else if (fromIndex < activeLayerIndex_ && toIndex >= activeLayerIndex_) {
        activeLayerIndex_--;
    } else if (fromIndex > activeLayerIndex_ && toIndex <= activeLayerIndex_) {
        activeLayerIndex_++;
    }
}

void GpuCanvas::duplicateLayer(size_t index) {
    if (!isValidLayerIndex(index)) {
        return;
    }

    // TODO: 实现 GPU 到 GPU 的图像复制
    BE_LOG_WARN("GpuCanvas::duplicateLayer: not yet implemented");
}

void GpuCanvas::mergeDown(size_t index) {
    if (index == 0 || index >= layers_.size()) {
        return;
    }

    // TODO: 使用合成管线将当前层渲染到下层
    BE_LOG_WARN("GpuCanvas::mergeDown: not yet implemented");
}

void GpuCanvas::flatten() {
    // TODO: 将所有可见图层合并到背景层
    BE_LOG_WARN("GpuCanvas::flatten: not yet implemented");
}

// ============================================================================
// 激活图层
// ============================================================================

void GpuCanvas::setActiveLayer(size_t index) {
    if (!isValidLayerIndex(index)) {
        BE_LOG_ERROR("GpuCanvas::setActiveLayer: invalid index {}", index);
        return;
    }
    activeLayerIndex_ = index;
}

// ============================================================================
// 图层属性访问
// ============================================================================

const std::string& GpuCanvas::layerName(size_t index) const {
    static const std::string empty;
    if (!isValidLayerIndex(index)) return empty;
    return layers_[index].name;
}

void GpuCanvas::setLayerName(size_t index, const std::string& name) {
    if (!isValidLayerIndex(index)) return;
    layers_[index].name = name;
}

float GpuCanvas::layerOpacity(size_t index) const {
    if (!isValidLayerIndex(index)) return 0.0f;
    return layers_[index].opacity;
}

void GpuCanvas::setLayerOpacity(size_t index, float opacity) {
    if (!isValidLayerIndex(index)) return;
    layers_[index].opacity = std::max(0.0f, std::min(1.0f, opacity));
}

uint32_t GpuCanvas::layerBlendMode(size_t index) const {
    if (!isValidLayerIndex(index)) return 0;
    return layers_[index].blendMode;
}

void GpuCanvas::setLayerBlendMode(size_t index, uint32_t mode) {
    if (!isValidLayerIndex(index)) return;
    layers_[index].blendMode = mode;
}

bool GpuCanvas::layerVisible(size_t index) const {
    if (!isValidLayerIndex(index)) return false;
    return layers_[index].visible;
}

void GpuCanvas::setLayerVisible(size_t index, bool visible) {
    if (!isValidLayerIndex(index)) return;
    layers_[index].visible = visible;
}

bool GpuCanvas::layerLocked(size_t index) const {
    if (!isValidLayerIndex(index)) return false;
    return layers_[index].locked;
}

void GpuCanvas::setLayerLocked(size_t index, bool locked) {
    if (!isValidLayerIndex(index)) return;
    layers_[index].locked = locked;
}

// ============================================================================
// GPU 资源访问
// ============================================================================

VkImageView GpuCanvas::layerImageView(size_t index) const {
    if (!isValidLayerIndex(index)) return VK_NULL_HANDLE;
    return layers_[index].view;
}

VkImage GpuCanvas::layerImage(size_t index) const {
    if (!isValidLayerIndex(index)) return VK_NULL_HANDLE;
    return layers_[index].image;
}

VkFramebuffer GpuCanvas::layerFramebuffer(size_t index) const {
    if (!isValidLayerIndex(index)) return VK_NULL_HANDLE;
    return layers_[index].framebuffer;
}

// ============================================================================
// 合成
// ============================================================================

uint32_t GpuCanvas::composite(VkCommandBuffer cmd, VkImageView outputView) {
    if (!compositeInitialized_) {
        BE_LOG_ERROR("GpuCanvas::composite: composite pipeline not initialized");
        return 0;
    }

    std::vector<VkImageView> views;
    std::vector<CompositeLayerParams> params;
    gatherVisibleLayers(views, params);

    if (views.empty()) {
        // 没有可见图层，只渲染背景色
        views.push_back(layers_[0].view);
        CompositeLayerParams dummyParam;
        dummyParam.opacity = 0.0f;
        dummyParam.blendMode = 0;
        params.push_back(dummyParam);
    }

    return recordCompositeRender(
        cmd,
        compositeState_,
        views,
        params,
        backgroundColor_,
        outputView,
        static_cast<uint32_t>(width_),
        static_cast<uint32_t>(height_)
    );
}

// ============================================================================
// 数据导入导出
// ============================================================================

void GpuCanvas::importLayerPixels(size_t index, const std::vector<uint8_t>& rgba8) {
    if (!isValidLayerIndex(index)) return;
    if (rgba8.size() != static_cast<size_t>(width_ * height_ * 4)) {
        BE_LOG_ERROR("GpuCanvas::importLayerPixels: data size mismatch");
        return;
    }

    // TODO: 创建 staging buffer + vkCmdCopyBufferToImage
    BE_LOG_WARN("GpuCanvas::importLayerPixels: not yet implemented");
}

std::vector<uint8_t> GpuCanvas::exportLayerPixels(size_t index) const {
    std::vector<uint8_t> result;
    if (!isValidLayerIndex(index)) return result;

    // TODO: 创建 staging buffer + vkCmdCopyImageToBuffer + vkMapMemory
    BE_LOG_WARN("GpuCanvas::exportLayerPixels: not yet implemented");
    return result;
}

// ============================================================================
// 内部方法
// ============================================================================

bool GpuCanvas::createGpuLayer(GpuLayer& layer) {
    // 1. 创建图像
    VkImageCreateInfo imageInfo = {};
    imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageInfo.imageType = VK_IMAGE_TYPE_2D;
    imageInfo.extent.width = static_cast<uint32_t>(width_);
    imageInfo.extent.height = static_cast<uint32_t>(height_);
    imageInfo.extent.depth = 1;
    imageInfo.mipLevels = 1;
    imageInfo.arrayLayers = 1;
    imageInfo.format = LAYER_FORMAT;
    imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
    imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    imageInfo.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;

    VK_CHECK(vkCreateImage(g_device, &imageInfo, nullptr, &layer.image));
    if (layer.image == VK_NULL_HANDLE) {
        return false;
    }

    // 2. 分配内存
    VkMemoryRequirements memReq;
    vkGetImageMemoryRequirements(g_device, layer.image, &memReq);

    VkMemoryAllocateInfo allocInfo = {};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memReq.size;
    allocInfo.memoryTypeIndex = findMemoryType(g_physicalDevice, memReq.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    VK_CHECK(vkAllocateMemory(g_device, &allocInfo, nullptr, &layer.memory));
    if (layer.memory == VK_NULL_HANDLE) {
        vkDestroyImage(g_device, layer.image, nullptr);
        layer.image = VK_NULL_HANDLE;
        return false;
    }

    VK_CHECK(vkBindImageMemory(g_device, layer.image, layer.memory, 0));

    // 3. 创建图像视图
    VkImageViewCreateInfo viewInfo = {};
    viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    viewInfo.image = layer.image;
    viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    viewInfo.format = LAYER_FORMAT;
    viewInfo.subresourceRange = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 };

    VK_CHECK(vkCreateImageView(g_device, &viewInfo, nullptr, &layer.view));
    if (layer.view == VK_NULL_HANDLE) {
        vkFreeMemory(g_device, layer.memory, nullptr);
        vkDestroyImage(g_device, layer.image, nullptr);
        layer.memory = VK_NULL_HANDLE;
        layer.image = VK_NULL_HANDLE;
        return false;
    }

    // 4. 创建 framebuffer（用于笔刷 render pass）
    // 需要共享的 render pass，这里暂时不创建，使用时动态创建
    // TODO: 使用共享的 brush render pass 创建 framebuffer

    return true;
}

void GpuCanvas::destroyGpuLayer(GpuLayer& layer) {
    if (layer.framebuffer != VK_NULL_HANDLE) {
        vkDestroyFramebuffer(g_device, layer.framebuffer, nullptr);
        layer.framebuffer = VK_NULL_HANDLE;
    }
    if (layer.view != VK_NULL_HANDLE) {
        vkDestroyImageView(g_device, layer.view, nullptr);
        layer.view = VK_NULL_HANDLE;
    }
    if (layer.image != VK_NULL_HANDLE) {
        vkDestroyImage(g_device, layer.image, nullptr);
        layer.image = VK_NULL_HANDLE;
    }
    if (layer.memory != VK_NULL_HANDLE) {
        vkFreeMemory(g_device, layer.memory, nullptr);
        layer.memory = VK_NULL_HANDLE;
    }
}

bool GpuCanvas::createLayerSampler() {
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
    samplerInfo.minLod = 0.0f;
    samplerInfo.maxLod = 1.0f;

    VK_CHECK(vkCreateSampler(g_device, &samplerInfo, nullptr, &layerSampler_));
    return layerSampler_ != VK_NULL_HANDLE;
}

void GpuCanvas::gatherVisibleLayers(std::vector<VkImageView>& outViews,
                                     std::vector<CompositeLayerParams>& outParams) const {
    outViews.clear();
    outParams.clear();

    for (size_t i = 0; i < layers_.size(); i++) {
        const auto& layer = layers_[i];
        if (!layer.visible) continue;

        outViews.push_back(layer.view);

        CompositeLayerParams param;
        param.opacity = layer.opacity;
        param.blendMode = layer.blendMode;
        outParams.push_back(param);
    }
}
