#include "tool_pen.h"
#include "../base/base_math.h"
#include <windows.h>

void ToolPen::onMouseDown(const InputSample& sample, Canvas& canvas, int button) {
    if (button != 0) return;
    
    Vec2 pos(sample.x, sample.y);
    
    if (path_.empty()) {
        // 第一个锚点
        AnchorPoint ap;
        ap.pos = pos;
        ap.outTangent = pos;
        path_.push_back(ap);
        drawing_ = true;
    } else {
        // 检查是否闭合
        if (path_.size() >= 2) {
            float dist = euclideanDistance(pos.x, pos.y, path_[0].pos.x, path_[0].pos.y);
            if (dist < 10.0f) {
                closePath();
                return;
            }
        }
        
        // 新锚点
        addAnchor(pos);
    }
}

void ToolPen::onMouseMove(const InputSample& sample, Canvas& canvas) {
    currentPos_ = Vec2(sample.x, sample.y);
    
    if (!path_.empty() && drawing_) {
        // 更新最后一个锚点的出切线
        auto& last = path_.back();
        last.outTangent = currentPos_;
        last.hasOutTangent = true;
    }
}

void ToolPen::onMouseUp(const InputSample& sample, Canvas& canvas, int button) {
    // 钢笔工具主要在 click 时处理
}

void ToolPen::onKeyDown(int keyCode, bool ctrl, bool shift, bool alt, Canvas& canvas) {
    if (keyCode == VK_ESCAPE) {
        path_.clear();
        drawing_ = false;
    }
}

void ToolPen::addAnchor(const Vec2& pos) {
    if (path_.empty()) return;
    
    // 前一个锚点的出切线成为当前锚点的入切线（对称）
    auto& prev = path_.back();
    Vec2 inTan = prev.pos * 2.0f - prev.outTangent; // 对称点
    
    AnchorPoint ap;
    ap.pos = pos;
    ap.inTangent = inTan;
    ap.hasInTangent = prev.hasOutTangent;
    ap.outTangent = pos;
    path_.push_back(ap);
}

void ToolPen::closePath() {
    if (path_.size() < 3) return;
    closingPath_ = true;
    drawing_ = false;
    // 这里可以将路径转为选区或填充
}

void ToolPen::drawBezier(CPURenderTarget& overlay, const AnchorPoint& a, const AnchorPoint& b, const Color& c) {
    // 使用 Catmull-Rom 或贝塞尔绘制
    // 简化：用 Catmull-Rom 近似
    Vec2 p0 = a.hasOutTangent ? a.outTangent : a.pos;
    Vec2 p1 = a.pos;
    Vec2 p2 = b.pos;
    Vec2 p3 = b.hasInTangent ? b.inTangent : b.pos;
    
    int segments = 50;
    for (int i = 0; i < segments; ++i) {
        float t1 = static_cast<float>(i) / segments;
        float t2 = static_cast<float>(i + 1) / segments;
        
        Vec2 pt1 = catmullRom(p0, p1, p2, p3, t1);
        Vec2 pt2 = catmullRom(p0, p1, p2, p3, t2);
        
        int x0 = static_cast<int>(pt1.x);
        int y0 = static_cast<int>(pt1.y);
        int x1 = static_cast<int>(pt2.x);
        int y1 = static_cast<int>(pt2.y);
        
        // 画线
        int dx = std::abs(x1 - x0), dy = std::abs(y1 - y0);
        int sx = x0 < x1 ? 1 : -1, sy = y0 < y1 ? 1 : -1;
        int err = dx - dy;
        while (true) {
            overlay.setPixelSafe(x0, y0, c);
            if (x0 == x1 && y0 == y1) break;
            int e2 = 2 * err;
            if (e2 > -dy) { err -= dy; x0 += sx; }
            if (e2 < dx) { err += dx; y0 += sy; }
        }
    }
}

void ToolPen::renderOverlay(CPURenderTarget& overlay, const RenderContext& ctx) {
    if (path_.empty()) return;
    
    Color pathC(0.3f, 0.6f, 1.0f, 1.0f);
    Color handleC(1.0f, 1.0f, 1.0f, 1.0f);
    Color tangentC(0.8f, 0.8f, 0.8f, 0.6f);
    
    // 绘制路径段
    for (size_t i = 1; i < path_.size(); ++i) {
        drawBezier(overlay, path_[i-1], path_[i], pathC);
    }
    
    // 绘制锚点
    for (const auto& ap : path_) {
        int x = static_cast<int>(ap.pos.x);
        int y = static_cast<int>(ap.pos.y);
        for (int dy = -3; dy <= 3; ++dy) {
            for (int dx = -3; dx <= 3; ++dx) {
                if (dx*dx + dy*dy <= 9) {
                    overlay.setPixelSafe(x + dx, y + dy, handleC);
                }
            }
        }
    }
    
    // 绘制切线控制点
    for (const auto& ap : path_) {
        if (ap.hasOutTangent) {
            int ax = static_cast<int>(ap.pos.x);
            int ay = static_cast<int>(ap.pos.y);
            int tx = static_cast<int>(ap.outTangent.x);
            int ty = static_cast<int>(ap.outTangent.y);
            
            // 连线
            int dx = std::abs(tx - ax), dy = std::abs(ty - ay);
            int sx = ax < tx ? 1 : -1, sy = ay < ty ? 1 : -1;
            int err = dx - dy;
            while (true) {
                overlay.setPixelSafe(ax, ay, tangentC);
                if (ax == tx && ay == ty) break;
                int e2 = 2 * err;
                if (e2 > -dy) { err -= dy; ax += sx; }
                if (e2 < dx) { err += dx; ay += sy; }
            }
            
            // 控制点
            for (int dy = -2; dy <= 2; ++dy) {
                for (int dx = -2; dx <= 2; ++dx) {
                    overlay.setPixelSafe(tx + dx, ty + dy, handleC);
                }
            }
        }
    }
    
    // 绘制到鼠标的预览线
    if (drawing_ && !path_.empty()) {
        auto& last = path_.back();
        int ax = static_cast<int>(last.pos.x);
        int ay = static_cast<int>(last.pos.y);
        int tx = static_cast<int>(currentPos_.x);
        int ty = static_cast<int>(currentPos_.y);
        
        int dx = std::abs(tx - ax), dy = std::abs(ty - ay);
        int sx = ax < tx ? 1 : -1, sy = ay < ty ? 1 : -1;
        int err = dx - dy;
        while (true) {
            overlay.setPixelSafe(ax, ay, tangentC);
            if (ax == tx && ay == ty) break;
            int e2 = 2 * err;
            if (e2 > -dy) { err -= dy; ax += sx; }
            if (e2 < dx) { err += dx; ay += sy; }
        }
    }
}