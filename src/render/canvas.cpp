#include "canvas.h"
#include <algorithm>

// 初始化画布尺寸、缩放、创建默认背景层
Canvas::Canvas(int width, int height)
    : width_(width), height_(height), canvasScale_(1.0f) {
    layers_.push_back(std::make_unique<Layer>(width, height, "Background"));
    activeLayerIndex_ = 0; // 默认激活第一层
}

// 获取指定索引图层
Layer& Canvas::layer(size_t index) {
    if (index >= layers_.size()) index = layers_.size() - 1;
    return *layers_[index];
}

// 获取指定索引图层
const Layer& Canvas::layer(size_t index) const {
    if (index >= layers_.size()) index = layers_.size() - 1;
    return *layers_[index];
}

// 设置当前激活图层
void Canvas::setActiveLayer(size_t index) {
    if (index < layers_.size()) {
        activeLayerIndex_ = index;
    }
}

// 添加新图层到末尾，返回新图层索引
size_t Canvas::addLayer(const std::string& name) {
    layers_.push_back(std::make_unique<Layer>(width_, height_, name));
    activeLayerIndex_ = layers_.size() - 1; // 新图层设为激活
    return activeLayerIndex_;
}

// 在指定索引位置插入新图层
size_t Canvas::addLayer(size_t index, const std::string& name) {
    if (index > layers_.size()) index = layers_.size(); // 索引越界修正
    auto it = layers_.begin() + index;
    layers_.insert(it, std::make_unique<Layer>(width_, height_, name));
    activeLayerIndex_ = index;
    return activeLayerIndex_;
}

// 删除指定图层（保留背景层）
void Canvas::removeLayer(size_t index) {
    if (layers_.size() <= 1) return; // 至少保留一层
    if (index >= layers_.size()) return;
    if (index == 0) return; // 禁止删除背景层

    layers_.erase(layers_.begin() + index);

    // 修正激活图层索引
    if (activeLayerIndex_ >= layers_.size()) {
        activeLayerIndex_ = layers_.size() - 1;
    }
    if (activeLayerIndex_ == index && index > 0) {
        activeLayerIndex_ = index - 1;
    }
}

// 移动图层位置
void Canvas::moveLayer(size_t fromIndex, size_t toIndex) {
    if (fromIndex >= layers_.size() || toIndex >= layers_.size()) return;
    if (fromIndex == toIndex) return;

    // 移动图层所有权
    auto layer = std::move(layers_[fromIndex]);
    layers_.erase(layers_.begin() + fromIndex);
    layers_.insert(layers_.begin() + toIndex, std::move(layer));

    // 更新激活图层索引
    if (activeLayerIndex_ == fromIndex) {
        activeLayerIndex_ = toIndex;
    } else if (fromIndex < toIndex) {
        if (activeLayerIndex_ > fromIndex && activeLayerIndex_ <= toIndex) {
            activeLayerIndex_--;
        }
    } else {
        if (activeLayerIndex_ >= toIndex && activeLayerIndex_ < fromIndex) {
            activeLayerIndex_++;
        }
    }
}

// 复制指定图层
void Canvas::duplicateLayer(size_t index) {
    if (index >= layers_.size()) return;

    const Layer& src = *layers_[index];
    // 复制到原图层下方
    size_t newIndex = addLayer(index + 1, src.name() + " Copy");
    Layer& dst = *layers_[newIndex];

    // 复制图层属性
    dst.setOpacity(src.opacity());
    dst.setBlendMode(src.blendMode());
    dst.setVisible(src.visible());

    // 复制像素数据
    const auto& srcPixels = src.renderTarget().pixels();
    auto& dstPixels = dst.renderTarget().pixels();
    std::copy(srcPixels.begin(), srcPixels.end(), dstPixels.begin());
}

// 向下合并图层
void Canvas::mergeDown(size_t index) {
    if (index == 0 || index >= layers_.size()) return;

    Layer& src = *layers_[index];
    Layer& dst = *layers_[index - 1];

    // 不可见则直接删除
    if (!src.visible()) {
        removeLayer(index);
        return;
    }

    // 混合参数
    BlendPipeline pipeline;
    pipeline.mode = src.blendMode();
    pipeline.opacity = src.opacity();

    // 逐像素混合
    for (int y = 0; y < height_; ++y) {
        for (int x = 0; x < width_; ++x) {
            Color srcColor = src.getPixel(x, y);
            Color dstColor = dst.getPixel(x, y);
            Color result = pipeline.apply(srcColor, dstColor);
            dst.setPixel(x, y, result);
        }
    }

    removeLayer(index); // 合并后删除原图层
}


// 合并所有图层为单层
void Canvas::flatten() {
    if (layers_.size() <= 1) return;

    CPURenderTarget accum(width_, height_);
    accum.clear(backgroundColor_);

    // 依次合成所有可见图层
    for (const auto& layerPtr : layers_) {
        const Layer& layer = *layerPtr;
        if (!layer.visible()) continue;
        compositeLayer(layer, accum);
    }

    // 重置为单背景层
    layers_.clear();
    layers_.push_back(std::make_unique<Layer>(width_, height_, "Background"));
    activeLayerIndex_ = 0;

    // 将合成结果写入背景层
    auto& bgPixels = backgroundLayer().renderTarget().pixels();
    const auto& accumPixels = accum.pixels();
    std::copy(accumPixels.begin(), accumPixels.end(), bgPixels.begin());
}

// 合成单个图层到目标缓冲区
void Canvas::compositeLayer(const Layer& layer, CPURenderTarget& accum) const {
    if (!layer.visible()) return;

    BlendPipeline pipeline;
    pipeline.mode = layer.blendMode();
    pipeline.opacity = layer.opacity();

    // 逐像素混合
    for (int y = 0; y < height_; ++y) {
        for (int x = 0; x < width_; ++x) {
            Color srcColor = layer.getPixel(x, y);
            Color dstColor = accum.getPixelSafe(x, y);
            Color result = pipeline.apply(srcColor, dstColor);
            accum.setPixelSafe(x, y, result);
        }
    }
}

// 整体合成画布到渲染目标
void Canvas::composite(RenderTarget& target) const {
    target.clear(backgroundColor_);

    // CPU渲染直接合成
    CPURenderTarget* cpuTarget = dynamic_cast<CPURenderTarget*>(&target);
    if (cpuTarget) {
        for (const auto& layerPtr : layers_) {
            compositeLayer(*layerPtr, *cpuTarget);
        }
        return;
    }

    // 先合成到临时CPU缓冲区
    CPURenderTarget temp(width_, height_);
    temp.clear(backgroundColor_);
    for (const auto& layerPtr : layers_) {
        compositeLayer(*layerPtr, temp);
    }

    for (int y = 0; y < height_; ++y) {
        for (int x = 0; x < width_; ++x) {
            target.setPixel(x, y, temp.getPixel(x, y));
        }
    }
}

// 获取指定坐标最终合成颜色
Color Canvas::getCompositedPixel(int x, int y) const {
    if (x < 0 || x >= width_ || y < 0 || y >= height_) {
        return Color(0, 0, 0, 0);
    }

    Color result = backgroundColor_;
    // 遍历所有图层混合计算
    for (const auto& layerPtr : layers_) {
        const Layer& layer = *layerPtr;
        if (!layer.visible()) continue;

        BlendPipeline pipeline;
        pipeline.mode = layer.blendMode();
        pipeline.opacity = layer.opacity();

        Color srcColor = layer.getPixel(x, y);
        result = pipeline.apply(srcColor, result);
    }
    return result;
}

// 导出画布为RGBA8字节向量
std::vector<uint8_t> Canvas::exportToRGBA8() const {
    std::vector<uint8_t> result;
    exportToRGBA8(result);
    return result;
}

// 导出RGBA8数据到输出向量
void Canvas::exportToRGBA8(std::vector<uint8_t>& out) const {
    CPURenderTarget temp(width_, height_);
    composite(temp);
    temp.exportToRGBA8(out);
}

// 从RGBA8数据导入到当前激活图层
void Canvas::importFromRGBA8(const std::vector<uint8_t>& data, float gamma) {
    if (data.size() < static_cast<size_t>(width_ * height_ * 4)) return;

    Layer& target = activeLayer();
    target.renderTarget().importFromRGBA8(data, gamma);
}
