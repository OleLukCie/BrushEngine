#pragma once

#include <imgui.h> 
#include "../base/base_types.h"
#include <string>
#include <vector>
#include <functional>

namespace UI {

// ============================================
// 通用矩形工具（保留，其他地方可能用到）
// ============================================
struct Rect {
    float x, y, w, h;

    bool contains(float px, float py) const {
        return px >= x && px < x + w && py >= y && py < y + h;
    }

    // 与 ImVec2/ImVec4 互转辅助
    static Rect fromImGui(const ImVec2& pos, const ImVec2& size) {
        return { pos.x, pos.y, size.x, size.y };
    }
};

// ============================================
// 颜色工具函数
// ============================================

// Color -> ImVec4
inline ImVec4 toImVec4(const Color& c) {
    return ImVec4(c.r, c.g, c.b, c.a);
}

// ImVec4 -> Color
inline Color fromImVec4(const ImVec4& v) {
    return Color(v.x, v.y, v.z, v.w);
}

// ============================================
// 画布视口辅助
// ============================================

// 计算画布在窗口中的居中显示区域
Rect calcCanvasViewport(int windowW, int windowH, int canvasW, int canvasH, float scale);

// 屏幕坐标 -> 画布坐标
Vec2 screenToCanvas(float screenX, float screenY, const Rect& viewport, float scale, const Vec2& offset);

// 画布坐标 -> 屏幕坐标
Vec2 canvasToScreen(float canvasX, float canvasY, const Rect& viewport, float scale, const Vec2& offset);

// ============================================
// ImGui 辅助：带图标的工具按钮
// ============================================

// 绘制一个方形工具按钮（用于工具栏）
// icon: 显示文字（如 "B"）
// tooltip: 悬停提示
// isActive: 是否高亮
// size: 按钮尺寸
bool ToolButton(const char* icon, const char* tooltip, bool isActive, ImVec2 size = ImVec2(36, 36));

// 绘制图层缩略图（ImGui Image 占位，实际绑定 Vulkan Texture）
void LayerThumbnail(ImTextureID textureID, ImVec2 size, bool isActive, bool isVisible);

// ============================================
// 颜色历史管理
// ============================================
class ColorHistory {
public:
    static constexpr int MAX_COLORS = 16;

    void push(const Color& c);
    const Color& get(int index) const;
    int count() const { return static_cast<int>(colors_.size()); }

    // 在 ImGui 中绘制颜色历史网格
    void drawImGui(float buttonSize = 20.0f);

private:
    std::vector<Color> colors_;
};

} // namespace UI