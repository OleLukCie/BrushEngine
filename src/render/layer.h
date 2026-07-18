#pragma once

#include "render_target.h"
#include "blend_mode.h"
#include <string>
#include <memory>

// 图层类
class Layer {
public:
    // 初始化图层宽、高、名称
    Layer(int width, int height, const std::string& name = "Layer");
    ~Layer() = default;

    // 禁用拷贝构造
    Layer(const Layer&) = delete;
    // 禁用拷贝赋值
    Layer& operator=(const Layer&) = delete;

    // 默认移动构造
    Layer(Layer&&) = default;
    // 默认移动赋值
    Layer& operator=(Layer&&) = default;

    // 获取图层名称
    const std::string& name() const { return name_; }
    // 设置图层名称
    void setName(const std::string& name) { name_ = name; }

    // 获取图层宽度
    int width() const { return renderTarget_->width(); }
    // 获取图层高度
    int height() const { return renderTarget_->height(); }

    // 获取图层可见性
    bool visible() const { return visible_; }
    // 设置图层可见性
    void setVisible(bool visible) { visible_ = visible; }

    // 获取图层不透明度
    float opacity() const { return opacity_; }
    // 设置图层不透明度
    void setOpacity(float opacity) { opacity_ = saturate(opacity); }

    // 获取图层混合模式
    BlendMode blendMode() const { return blendMode_; }
    // 设置图层混合模式
    void setBlendMode(BlendMode mode) { blendMode_ = mode; }

    // 获取渲染目标
    CPURenderTarget& renderTarget() { return *renderTarget_; }
    const CPURenderTarget& renderTarget() const { return *renderTarget_; }

    // 获取指定像素颜色
    Color getPixel(int x, int y) const { return renderTarget_->getPixelSafe(x, y); }
    // 设置指定像素颜色
    void setPixel(int x, int y, const Color& color) { renderTarget_->setPixelSafe(x, y, color); }

    // 清空图层（默认透明黑）
    void clear(const Color& color = Color(0, 0, 0, 0)) { renderTarget_->clear(color); }

    // 获取图层锁定状态
    bool locked() const { return locked_; }
    // 设置图层锁定状态
    void setLocked(bool locked) { locked_ = locked; }

private:
    // 图层名称
    std::string name_;
    // 渲染目标智能指针
    std::unique_ptr<CPURenderTarget> renderTarget_;
    // 图层可见性（默认可见）
    bool visible_ = true;
    // 图层不透明度（默认完全不透明）
    float opacity_ = 1.0f;
    // 混合模式（默认正常）
    BlendMode blendMode_ = BlendMode::Normal;
    // 图层锁定状态（默认未锁定）
    bool locked_ = false;
};
