#include "tool_lasso.h"

void ToolLasso::onMouseDown(const InputSample& sample, Canvas& canvas, int button) {
    if (button != 0) return;
    drawing_ = true;
    points_.clear();
    points_.push_back(Vec2(sample.x, sample.y));
    currentPos_ = points_.back();
}

void ToolLasso::onMouseMove(const InputSample& sample, Canvas& canvas) {
    if (!drawing_) return;
    currentPos_ = Vec2(sample.x, sample.y);
    
    float dist = euclideanDistance(points_.back().x, points_.back().y, sample.x, sample.y);
    if (dist > 3.0f) {
        points_.push_back(currentPos_);
    }
}

void ToolLasso::onMouseUp(const InputSample& sample, Canvas& canvas, int button) {
    if (button != 0 || !drawing_) return;
    drawing_ = false;
    
    // 闭合路径
    if (points_.size() >= 3) {
        points_.push_back(points_.front());
        hasSelection_ = true;
    } else {
        points_.clear();
        hasSelection_ = false;
    }
}

void ToolLasso::renderOverlay(CPURenderTarget& overlay, const RenderContext& ctx) {
    if (points_.size() < 2) return;
    
    Color c = drawing_ ? Color(0.3f, 0.6f, 1.0f, 0.8f) : Color(1, 1, 1, 0.8f);
    
    // 绘制路径
    for (size_t i = 1; i < points_.size(); ++i) {
        // Bresenham 画线
        int x0 = static_cast<int>(points_[i-1].x);
        int y0 = static_cast<int>(points_[i-1].y);
        int x1 = static_cast<int>(points_[i].x);
        int y1 = static_cast<int>(points_[i].y);
        
        int dx = std::abs(x1 - x0);
        int dy = std::abs(y1 - y0);
        int sx = x0 < x1 ? 1 : -1;
        int sy = y0 < y1 ? 1 : -1;
        int err = dx - dy;
        
        while (true) {
            overlay.setPixelSafe(x0, y0, c);
            if (x0 == x1 && y0 == y1) break;
            int e2 = 2 * err;
            if (e2 > -dy) { err -= dy; x0 += sx; }
            if (e2 < dx) { err += dx; y0 += sy; }
        }
    }
    
    // 绘制到鼠标的临时线
    if (drawing_ && !points_.empty()) {
        int x0 = static_cast<int>(points_.back().x);
        int y0 = static_cast<int>(points_.back().y);
        int x1 = static_cast<int>(currentPos_.x);
        int y1 = static_cast<int>(currentPos_.y);
        
        int dx = std::abs(x1 - x0);
        int dy = std::abs(y1 - y0);
        int sx = x0 < x1 ? 1 : -1;
        int sy = y0 < y1 ? 1 : -1;
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