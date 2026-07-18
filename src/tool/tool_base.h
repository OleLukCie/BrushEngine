#pragma once

#include "../sys/tool_type.h"
#include "../sys/input_sample.h"
#include "../sys/brush.h" 
#include "../render/canvas.h"
#include "../base/base_types.h"
#include <string>

// 工具基类：所有非笔刷工具的抽象接口
class Tool {
public:
    Tool() = default;
    virtual ~Tool() = default;

    // 禁用拷贝
    Tool(const Tool&) = delete;
    Tool& operator=(const Tool&) = delete;
    Tool(Tool&&) = default;
    Tool& operator=(Tool&&) = default;

    // ===== 核心接口 =====

    // 工具标识
    virtual ToolType type() const = 0;
    virtual const char* name() const = 0;
    virtual const char* icon() const = 0; // 单字符图标，用于工具栏

    // 生命周期
    virtual void onActivate(Canvas& canvas) {}   // 切换到该工具时
    virtual void onDeactivate(Canvas& canvas) {} // 离开该工具时

    // 输入事件
    virtual void onMouseDown(const InputSample& sample, Canvas& canvas, int button) = 0;
    virtual void onMouseMove(const InputSample& sample, Canvas& canvas) = 0;
    virtual void onMouseUp(const InputSample& sample, Canvas& canvas, int button) = 0;
    virtual void onMouseWheel(float delta, Canvas& canvas) {} // 滚轮缩放等

    // 键盘事件（快捷键透传）
    virtual void onKeyDown(int keyCode, bool ctrl, bool shift, bool alt, Canvas& canvas) {}
    virtual void onKeyUp(int keyCode, Canvas& canvas) {}

    // 渲染（绘制工具预览/辅助线，在画布合成后调用）
    virtual void renderOverlay(CPURenderTarget& overlay, const RenderContext& ctx) {}

    // 状态查询
    virtual bool isActive() const { return active_; }
    virtual bool isDragging() const { return dragging_; }
    virtual bool hasSelection() const { return false; }

    // 光标类型提示（供系统设置鼠标光标）
    virtual const char* cursorHint() const { return "arrow"; }

protected:
    bool active_ = false;   // 工具是否处于操作状态
    bool dragging_ = false; // 是否正在拖拽

    // 辅助：坐标转换（画布坐标 <-> 屏幕坐标）
    Vec2 toCanvasSpace(const InputSample& sample, const RenderContext& ctx) const;
    Vec2 toScreenSpace(const Vec2& canvasPos, const RenderContext& ctx) const;
};