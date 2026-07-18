#pragma once

#include "tool_base.h"

// 文字工具：点击放置文本框，支持简单编辑
class ToolText : public Tool {
public:
    ToolText() = default;
    ~ToolText() override = default;

    ToolType type() const override { return ToolType::Text; }
    const char* name() const override { return "Type"; }
    const char* icon() const override { return "T"; }
    const char* cursorHint() const override { return "ibeam"; }

    void onMouseDown(const InputSample& sample, Canvas& canvas, int button) override;
    void onMouseMove(const InputSample& sample, Canvas& canvas) override;
    void onMouseUp(const InputSample& sample, Canvas& canvas, int button) override;

    void onKeyDown(int keyCode, bool ctrl, bool shift, bool alt, Canvas& canvas) override;

    void renderOverlay(CPURenderTarget& overlay, const RenderContext& ctx) override;

    // 文本设置
    void setFontSize(float size) { fontSize_ = size; }
    float fontSize() const { return fontSize_; }

    void setTextColor(const Color& c) { textColor_ = c; }
    Color textColor() const { return textColor_; }

    // 提交文本到图层
    void commitText(Canvas& canvas);
    void cancelText();

private:
    bool placing_ = false;
    bool editing_ = false;
    Vec2 textPos_;
    std::string text_;
    float fontSize_ = 24.0f;
    Color textColor_ = Color(0, 0, 0, 1);
    
    // 光标闪烁
    float cursorTimer_ = 0.0f;
    bool cursorVisible_ = true;
};