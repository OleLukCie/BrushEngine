#pragma once

#include "layer.h"
#include "blend_mode.h"
#include "texture.h"
#include <vector>
#include <memory>
#include <cstdint> // 标准整数类型

// 画布类
class Canvas {
public:
    Canvas(int width, int height); // 初始化画布宽高
    ~Canvas() = default;

    // 获取画布宽度
    int width() const { return width_; }
    // 获取画布高度
    int height() const { return height_; }

    // 获取图层数量
    size_t layerCount() const { return layers_.size(); }

    // 获取指定索引的图层
    Layer& layer(size_t index);
    const Layer& layer(size_t index) const;

    // 获取激活图层
    Layer& activeLayer() { return *layers_[activeLayerIndex_]; }
    const Layer& activeLayer() const { return *layers_[activeLayerIndex_]; }
    // 获取激活图层索引
    size_t activeLayerIndex() const { return activeLayerIndex_; }
    // 设置激活图层索引
    void setActiveLayer(size_t index);

    // 在尾部添加图层，默认名称Layer
    size_t addLayer(const std::string& name = "Layer");
    // 在指定位置添加图层，默认名称Layer
    size_t addLayer(size_t index, const std::string& name = "Layer");

    // 删除指定索引的图层
    void removeLayer(size_t index);

    // 移动图层
    void moveLayer(size_t fromIndex, size_t toIndex);

    // 复制指定索引的图层
    void duplicateLayer(size_t index);

    // 将指定图层向下合并
    void mergeDown(size_t index);

    // 合并所有图层到背景层
    void flatten();

    // 获取背景图层
    Layer& backgroundLayer() { return *layers_[0]; }
    const Layer& backgroundLayer() const { return *layers_[0]; }

    // 将所有图层合成为最终图像
    void composite(RenderTarget& target) const;

    // 导出画布为RGBA8格式数据（返回值）
    std::vector<uint8_t> exportToRGBA8() const;
    // 导出画布为RGBA8格式数据（输出参数）
    void exportToRGBA8(std::vector<uint8_t>& out) const;

    // 从RGBA8数据导入画布，默认gamma=2.2
    void importFromRGBA8(const std::vector<uint8_t>& data, float gamma = 2.2f);

    // 获取画布合成后指定坐标的像素颜色
    Color getCompositedPixel(int x, int y) const;

    // 获取画布缩放比例
    float canvasScale() const { return canvasScale_; }
    // 设置画布缩放比例
    void setCanvasScale(float scale) { canvasScale_ = scale; }

    // 获取画布背景色
    Color backgroundColor() const { return backgroundColor_; }
    // 设置画布背景色
    void setBackgroundColor(const Color& color) { backgroundColor_ = color; }

private:
    int width_; // 画布宽度
    int height_; // 画布高度
    std::vector<std::unique_ptr<Layer>> layers_; // 图层容器
    size_t activeLayerIndex_ = 0; // 激活图层索引，默认第0层
    float canvasScale_ = 1.0f; // 画布缩放比例，默认1.0
    Color backgroundColor_ = Color(1, 1, 1, 1); // 画布背景色，默认白色不透明

    // 合成单个图层到累加渲染目标
    void compositeLayer(const Layer& layer, CPURenderTarget& accum) const;
};
