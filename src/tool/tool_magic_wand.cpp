#include "tool_magic_wand.h"
#include "../base/base_math.h"
#include <queue>

void ToolMagicWand::onMouseDown(const InputSample& sample, Canvas& canvas, int button) {
    if (button != 0) return;
    
    int x = static_cast<int>(sample.x);
    int y = static_cast<int>(sample.y);
    
    if (x < 0 || x >= canvas.width() || y < 0 || y >= canvas.height()) return;
    
    floodFillSelect(x, y, canvas);
}

void ToolMagicWand::onMouseMove(const InputSample& sample, Canvas& canvas) {
    // 魔棒不跟踪移动
}

void ToolMagicWand::onMouseUp(const InputSample& sample, Canvas& canvas, int button) {
    // 已在 onMouseDown 完成选择
}

void ToolMagicWand::floodFillSelect(int startX, int startY, Canvas& canvas) {
    int w = canvas.width();
    int h = canvas.height();
    
    selectionMask_.assign(w * h, false);
    maskW_ = w;
    maskH_ = h;
    
    Color targetColor = canvas.getCompositedPixel(startX, startY);
    
    if (contiguous_) {
        // 泛洪填充
        std::queue<std::pair<int, int>> q;
        q.push({startX, startY});
        selectionMask_[startY * w + startX] = true;
        
        int dx4[4] = {0, 0, -1, 1};
        int dy4[4] = {-1, 1, 0, 0};
        
        while (!q.empty()) {
            auto [cx, cy] = q.front(); q.pop();
            
            for (int i = 0; i < 4; ++i) {
                int nx = cx + dx4[i];
                int ny = cy + dy4[i];
                
                if (nx < 0 || nx >= w || ny < 0 || ny >= h) continue;
                if (selectionMask_[ny * w + nx]) continue;
                
                Color c = canvas.getCompositedPixel(nx, ny);
                if (colorMatch(c, targetColor)) {
                    selectionMask_[ny * w + nx] = true;
                    q.push({nx, ny});
                }
            }
        }
    } else {
        // 全图匹配
        for (int y = 0; y < h; ++y) {
            for (int x = 0; x < w; ++x) {
                Color c = canvas.getCompositedPixel(x, y);
                if (colorMatch(c, targetColor)) {
                    selectionMask_[y * w + x] = true;
                }
            }
        }
    }
}

bool ToolMagicWand::colorMatch(const Color& a, const Color& b) const {
    float dr = a.r - b.r;
    float dg = a.g - b.g;
    float db = a.b - b.b;
    float da = a.a - b.a;
    float dist = std::sqrt(dr*dr + dg*dg + db*db + da*da) * 0.5f; // 归一化到 [0,1]
    return dist <= tolerance_;
}

void ToolMagicWand::renderOverlay(CPURenderTarget& overlay, const RenderContext& ctx) {
    if (selectionMask_.empty()) return;
    
    Color c(0.3f, 0.6f, 1.0f, 0.3f);
    for (int y = 0; y < maskH_; ++y) {
        for (int x = 0; x < maskW_; ++x) {
            if (selectionMask_[y * maskW_ + x]) {
                overlay.setPixelSafe(x, y, c);
            }
        }
    }
}