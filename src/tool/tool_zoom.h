#pragma once

#include "tool_base.h"
#include <functional>

// 缩放工具：点击放大，Alt+点击缩小，拖拽框选放大区域
class ToolZoom : public Tool {
public:
    ToolZoom() = default;
    ~ToolZoom() override = default;

    ToolType type() const override { return ToolType::Zoom; }
    const char* name() const override { return "Zoom"; }
    const char* icon() const override { return "Z"; }
    const char* cursorHint() const override { return "zoom-in"; }

    void onMouseDown(const InputSample& sample, Canvas& canvas, int button) override;
    void onMouseMove(const InputSample& sample, Canvas& canvas) override;
    void onMouseUp(const InputSample& sample, Canvas& canvas, int button) override;
    void onMouseWheel(float delta, Canvas& canvas) override;

    void renderOverlay(CPURenderTarget& overlay, const RenderContext& ctx) override;

    // 缩放回调（通知视图系统）
    using ZoomCallback = std::function<void(float scale, const Vec2& center)>;
    void setOnZoom(ZoomCallback cb) { onZoom_ = cb; }

private:
    bool zoomOut_ = false;      // Alt 按下时缩小
    bool dragging_ = false;     // 框选模式
    Vec2 dragStart_;
    Vec2 dragCurrent_;
    ZoomCallback onZoom_;
    
    static constexpr float ZOOM_STEP = 1.25f;
};