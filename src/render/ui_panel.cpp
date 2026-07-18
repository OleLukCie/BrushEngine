#include "ui_panel.h"
#include "../base/base_math.h"
#include <imgui.h>

namespace UI {

// ============================================
// 画布视口计算
// ============================================

Rect calcCanvasViewport(int windowW, int windowH, int canvasW, int canvasH, float scale) {
    float scaledW = canvasW * scale;
    float scaledH = canvasH * scale;

    float x = (windowW - scaledW) * 0.5f;
    float y = (windowH - scaledH) * 0.5f;

    return { x, y, scaledW, scaledH };
}

Vec2 screenToCanvas(float screenX, float screenY, const Rect& viewport, float scale, const Vec2& offset) {
    float localX = (screenX - viewport.x) / scale;
    float localY = (screenY - viewport.y) / scale;
    return Vec2(localX - offset.x, localY - offset.y);
}

Vec2 canvasToScreen(float canvasX, float canvasY, const Rect& viewport, float scale, const Vec2& offset) {
    float screenX = viewport.x + (canvasX + offset.x) * scale;
    float screenY = viewport.y + (canvasY + offset.y) * scale;
    return Vec2(screenX, screenY);
}

// ============================================
// 工具按钮
// ============================================

bool ToolButton(const char* icon, const char* tooltip, bool isActive, ImVec2 size) {
    ImGuiStyle& style = ImGui::GetStyle();

    if (isActive) {
        ImGui::PushStyleColor(ImGuiCol_Button, style.Colors[ImGuiCol_HeaderActive]);
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, style.Colors[ImGuiCol_HeaderHovered]);
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, style.Colors[ImGuiCol_HeaderActive]);
    }

    bool clicked = ImGui::Button(icon, size);

    if (isActive) {
        ImGui::PopStyleColor(3);
    }

    if (ImGui::IsItemHovered()) {
        ImGui::SetTooltip("%s", tooltip);
    }

    return clicked;
}

// ============================================
// 图层缩略图（占位，后续绑定 Vulkan Texture）
// ============================================

void LayerThumbnail(ImTextureID textureID, ImVec2 size, bool isActive, bool isVisible) {
    ImVec2 cursor = ImGui::GetCursorScreenPos();

    // 背景
    ImU32 bgColor = isActive ?
        ImGui::GetColorU32(ImGuiCol_HeaderActive) :
        ImGui::GetColorU32(ImGuiCol_FrameBg);

    ImGui::GetWindowDrawList()->AddRectFilled(cursor, ImVec2(cursor.x + size.x, cursor.y + size.y), bgColor, 4.0f);

    // 如果有纹理，绘制纹理
    if (textureID) {
        ImGui::Image(textureID, size);
    } else {
        // 占位：显示棋盘格
        const int checkerSize = 8;
        for (int y = 0; y < static_cast<int>(size.y); y += checkerSize) {
            for (int x = 0; x < static_cast<int>(size.x); x += checkerSize) {
                bool dark = ((x / checkerSize) + (y / checkerSize)) % 2 == 0;
                ImU32 col = dark ? IM_COL32(80, 80, 80, 255) : IM_COL32(120, 120, 120, 255);
                ImVec2 p1(cursor.x + x, cursor.y + y);
                ImVec2 p2(
                    std::min(cursor.x + x + checkerSize, cursor.x + size.x),
                    std::min(cursor.y + y + checkerSize, cursor.y + size.y)
                );
                ImGui::GetWindowDrawList()->AddRectFilled(p1, p2, col);
            }
        }
    }

    // 可见性指示器
    if (!isVisible) {
        ImU32 overlay = IM_COL32(0, 0, 0, 128);
        ImGui::GetWindowDrawList()->AddRectFilled(cursor, ImVec2(cursor.x + size.x, cursor.y + size.y), overlay);
    }

    ImGui::Dummy(size);
}

// ============================================
// 颜色历史
// ============================================

void ColorHistory::push(const Color& c) {
    // 去重：如果已经在顶部，不重复添加
    if (!colors_.empty()) {
        const Color& top = colors_.back();
        if (std::abs(top.r - c.r) < 0.001f &&
            std::abs(top.g - c.g) < 0.001f &&
            std::abs(top.b - c.b) < 0.001f &&
            std::abs(top.a - c.a) < 0.001f) {
            return;
        }
    }

    colors_.push_back(c);
    if (colors_.size() > MAX_COLORS) {
        colors_.erase(colors_.begin());
    }
}

const Color& ColorHistory::get(int index) const {
    static Color fallback(0, 0, 0, 1);
    if (index < 0 || index >= static_cast<int>(colors_.size())) return fallback;
    return colors_[index];
}

void ColorHistory::drawImGui(float buttonSize) {
    int count = this->count();
    if (count == 0) {
        ImGui::TextDisabled("No history");
        return;
    }

    float availWidth = ImGui::GetContentRegionAvail().x;
    int buttonsPerRow = std::max(1, static_cast<int>(availWidth / (buttonSize + 4)));

    for (int i = count - 1; i >= 0; --i) {
        if (i < count - 1 && (count - 1 - i) % buttonsPerRow != 0) {
            ImGui::SameLine();
        }

        const Color& c = colors_[i];
        ImVec4 imColor(c.r, c.g, c.b, c.a);

        ImGui::PushID(i);
        if (ImGui::ColorButton("##history", imColor, ImGuiColorEditFlags_NoTooltip, ImVec2(buttonSize, buttonSize))) {
            // 点击选中
        }
        if (ImGui::IsItemHovered()) {
            ImGui::SetTooltip("RGB: %.0f, %.0f, %.0f", c.r * 255, c.g * 255, c.b * 255);
        }
        ImGui::PopID();
    }
}

} // namespace UI