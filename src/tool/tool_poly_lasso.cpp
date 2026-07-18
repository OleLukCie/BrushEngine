#include "tool_poly_lasso.h"
#include <windows.h>

void ToolPolyLasso::onMouseDown(const InputSample& sample, Canvas& canvas, int button) {
    if (button != 0) return;
    
    if (closed_) {
        vertices_.clear();
        closed_ = false;
        hasSelection_ = false;
    }
    
    vertices_.push_back(Vec2(sample.x, sample.y));
}

void ToolPolyLasso::onMouseMove(const InputSample& sample, Canvas& canvas) {
    currentPos_ = Vec2(sample.x, sample.y);
}

void ToolPolyLasso::onMouseUp(const InputSample& sample, Canvas& canvas, int button) {
    // 多边形套索在点击时添加顶点，不在 mouseUp 处理
}

void ToolPolyLasso::onKeyDown(int keyCode, bool ctrl, bool shift, bool alt, Canvas& canvas) {
    if (keyCode == VK_RETURN && vertices_.size() >= 3) {
        // 回车闭合
        closed_ = true;
        hasSelection_ = true;
    }
    if (keyCode == VK_ESCAPE) {
        vertices_.clear();
        closed_ = false;
        hasSelection_ = false;
    }
}

void ToolPolyLasso::renderOverlay(CPURenderTarget& overlay, const RenderContext& ctx) {
    if (vertices_.empty()) return;
    
    Color lineC = Color(0.3f, 0.6f, 1.0f, 0.8f);
    Color vertexC = Color(1.0f, 1.0f, 1.0f, 1.0f);
    
    // 绘制已确定的边
    for (size_t i = 1; i < vertices_.size(); ++i) {
        int x0 = static_cast<int>(vertices_[i-1].x);
        int y0 = static_cast<int>(vertices_[i-1].y);
        int x1 = static_cast<int>(vertices_[i].x);
        int y1 = static_cast<int>(vertices_[i].y);
        
        // 简化画线
        int dx = std::abs(x1 - x0), dy = std::abs(y1 - y0);
        int sx = x0 < x1 ? 1 : -1, sy = y0 < y1 ? 1 : -1;
        int err = dx - dy;
        while (true) {
            overlay.setPixelSafe(x0, y0, lineC);
            if (x0 == x1 && y0 == y1) break;
            int e2 = 2 * err;
            if (e2 > -dy) { err -= dy; x0 += sx; }
            if (e2 < dx) { err += dx; y0 += sy; }
        }
    }
    
    // 绘制顶点小方块
    for (const auto& v : vertices_) {
        int x = static_cast<int>(v.x);
        int y = static_cast<int>(v.y);
        for (int dy = -2; dy <= 2; ++dy) {
            for (int dx = -2; dx <= 2; ++dx) {
                overlay.setPixelSafe(x + dx, y + dy, vertexC);
            }
        }
    }
    
    // 绘制到鼠标的临时线
    if (!vertices_.empty() && !closed_) {
        int x0 = static_cast<int>(vertices_.back().x);
        int y0 = static_cast<int>(vertices_.back().y);
        int x1 = static_cast<int>(currentPos_.x);
        int y1 = static_cast<int>(currentPos_.y);
        
        int dx = std::abs(x1 - x0), dy = std::abs(y1 - y0);
        int sx = x0 < x1 ? 1 : -1, sy = y0 < y1 ? 1 : -1;
        int err = dx - dy;
        while (true) {
            overlay.setPixelSafe(x0, y0, Color(0.3f, 0.6f, 1.0f, 0.5f));
            if (x0 == x1 && y0 == y1) break;
            int e2 = 2 * err;
            if (e2 > -dy) { err -= dy; x0 += sx; }
            if (e2 < dx) { err += dx; y0 += sy; }
        }
    }
}