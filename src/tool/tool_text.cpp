#define NOMINMAX
#include "tool_text.h"
#include <windows.h>
#include <algorithm>

void ToolText::onMouseDown(const InputSample& sample, Canvas& canvas, int button) {
    if (button != 0) return;

    if (!editing_) {
        // 放置文本框
        textPos_ = Vec2(sample.x, sample.y);
        placing_ = true;
        editing_ = true;
        text_.clear();
    }
}

void ToolText::onMouseMove(const InputSample& sample, Canvas& canvas) {
    if (placing_) {
        // 拖拽定义文本框宽度（简化：固定宽度）
    }
}

void ToolText::onMouseUp(const InputSample& sample, Canvas& canvas, int button) {
    placing_ = false;
}

void ToolText::onKeyDown(int keyCode, bool ctrl, bool shift, bool alt, Canvas& canvas) {
    if (!editing_) return;

    if (keyCode == VK_RETURN) {
        if (shift) {
            text_ += '\n';
        } else {
            commitText(canvas);
        }
    } else if (keyCode == VK_ESCAPE) {
        cancelText();
    } else if (keyCode == VK_BACK) {
        if (!text_.empty()) text_.pop_back();
    } else if (keyCode >= ' ' && keyCode <= '~') {
        char c = static_cast<char>(keyCode);
        if (shift) {
            // 简化：仅处理字母大写
            if (c >= 'a' && c <= 'z') c = c - 'a' + 'A';
        }
        text_ += c;
    }
}

static void drawCharToTarget(CPURenderTarget& target, char c, int x, int y, 
                              const Color& color, float fontSize);

void ToolText::commitText(Canvas& canvas) {
    if (text_.empty()) {
        editing_ = false;
        return;
    }

    // 将文本渲染到当前图层
    auto& layer = canvas.activeLayer();
    int x = static_cast<int>(textPos_.x);
    int y = static_cast<int>(textPos_.y);

    for (char c : text_) {
        if (c == '\n') {
            // 换行
            x = static_cast<int>(textPos_.x);
            y += static_cast<int>(fontSize_);
            continue;
        }

        // 简化：使用点阵字体逐字符绘制到图层像素
        drawCharToTarget(layer.renderTarget(), c, x, y, textColor_, fontSize_);
        x += static_cast<int>(fontSize_ * 0.6f);
    }

    editing_ = false;
    text_.clear();
}

void ToolText::cancelText() {
    editing_ = false;
    text_.clear();
}

// 简化的点阵字体：5x7 点阵定义（0 = 空, 1 = 有像素）
// 覆盖 ASCII 32-126 全部可打印字符
static const uint8_t FONT_5X7[][5] = {
    // 空格 ' ' (32)
    {0x00, 0x00, 0x00, 0x00, 0x00},
    // ! (33)
    {0x00, 0x00, 0x5F, 0x00, 0x00},
    // " (34)
    {0x00, 0x07, 0x00, 0x07, 0x00},
    // # (35)
    {0x14, 0x7F, 0x14, 0x7F, 0x14},
    // $ (36)
    {0x24, 0x2A, 0x7F, 0x2A, 0x12},
    // % (37)
    {0x23, 0x13, 0x08, 0x64, 0x62},
    // & (38)
    {0x36, 0x49, 0x55, 0x22, 0x50},
    // ' (39)
    {0x00, 0x05, 0x03, 0x00, 0x00},
    // ( (40)
    {0x00, 0x1C, 0x22, 0x41, 0x00},
    // ) (41)
    {0x00, 0x41, 0x22, 0x1C, 0x00},
    // * (42)
    {0x08, 0x2A, 0x1C, 0x2A, 0x08},
    // + (43)
    {0x08, 0x08, 0x3E, 0x08, 0x08},
    // , (44)
    {0x00, 0x50, 0x30, 0x00, 0x00},
    // - (45)
    {0x08, 0x08, 0x08, 0x08, 0x08},
    // . (46)
    {0x00, 0x60, 0x60, 0x00, 0x00},
    // / (47)
    {0x20, 0x10, 0x08, 0x04, 0x02},
    // 0 (48)
    {0x3E, 0x51, 0x49, 0x45, 0x3E},
    // 1 (49)
    {0x00, 0x42, 0x7F, 0x40, 0x00},
    // 2 (50)
    {0x42, 0x61, 0x51, 0x49, 0x46},
    // 3 (51)
    {0x21, 0x41, 0x45, 0x4B, 0x31},
    // 4 (52)
    {0x18, 0x14, 0x12, 0x7F, 0x10},
    // 5 (53)
    {0x27, 0x45, 0x45, 0x45, 0x39},
    // 6 (54)
    {0x3C, 0x4A, 0x49, 0x49, 0x30},
    // 7 (55)
    {0x01, 0x71, 0x09, 0x05, 0x03},
    // 8 (56)
    {0x36, 0x49, 0x49, 0x49, 0x36},
    // 9 (57)
    {0x06, 0x49, 0x49, 0x29, 0x1E},
    // : (58)
    {0x00, 0x36, 0x36, 0x00, 0x00},
    // ; (59)
    {0x00, 0x56, 0x36, 0x00, 0x00},
    // < (60)
    {0x00, 0x08, 0x14, 0x22, 0x41},
    // = (61)
    {0x14, 0x14, 0x14, 0x14, 0x14},
    // > (62)
    {0x41, 0x22, 0x14, 0x08, 0x00},
    // ? (63)
    {0x02, 0x01, 0x51, 0x09, 0x06},
    // @ (64)
    {0x32, 0x49, 0x79, 0x41, 0x3E},
    // A (65)
    {0x7E, 0x11, 0x11, 0x11, 0x7E},
    // B (66)
    {0x7F, 0x49, 0x49, 0x49, 0x36},
    // C (67)
    {0x3E, 0x41, 0x41, 0x41, 0x22},
    // D (68)
    {0x7F, 0x41, 0x41, 0x22, 0x1C},
    // E (69)
    {0x7F, 0x49, 0x49, 0x49, 0x41},
    // F (70)
    {0x7F, 0x09, 0x09, 0x01, 0x01},
    // G (71)
    {0x3E, 0x41, 0x41, 0x51, 0x32},
    // H (72)
    {0x7F, 0x08, 0x08, 0x08, 0x7F},
    // I (73)
    {0x00, 0x41, 0x7F, 0x41, 0x00},
    // J (74)
    {0x20, 0x40, 0x41, 0x3F, 0x01},
    // K (75)
    {0x7F, 0x08, 0x14, 0x22, 0x41},
    // L (76)
    {0x7F, 0x40, 0x40, 0x40, 0x40},
    // M (77)
    {0x7F, 0x02, 0x04, 0x02, 0x7F},
    // N (78)
    {0x7F, 0x04, 0x08, 0x10, 0x7F},
    // O (79)
    {0x3E, 0x41, 0x41, 0x41, 0x3E},
    // P (80)
    {0x7F, 0x09, 0x09, 0x09, 0x06},
    // Q (81)
    {0x3E, 0x41, 0x51, 0x21, 0x5E},
    // R (82)
    {0x7F, 0x09, 0x19, 0x29, 0x46},
    // S (83)
    {0x46, 0x49, 0x49, 0x49, 0x31},
    // T (84)
    {0x01, 0x01, 0x7F, 0x01, 0x01},
    // U (85)
    {0x3F, 0x40, 0x40, 0x40, 0x3F},
    // V (86)
    {0x1F, 0x20, 0x40, 0x20, 0x1F},
    // W (87)
    {0x7F, 0x20, 0x18, 0x20, 0x7F},
    // X (88)
    {0x63, 0x14, 0x08, 0x14, 0x63},
    // Y (89)
    {0x03, 0x04, 0x78, 0x04, 0x03},
    // Z (90)
    {0x61, 0x51, 0x49, 0x45, 0x43},
    // [ (91)
    {0x00, 0x00, 0x7F, 0x41, 0x41},
    // \ (92)
    {0x02, 0x04, 0x08, 0x10, 0x20},
    // ] (93)
    {0x41, 0x41, 0x7F, 0x00, 0x00},
    // ^ (94)
    {0x04, 0x02, 0x01, 0x02, 0x04},
    // _ (95)
    {0x40, 0x40, 0x40, 0x40, 0x40},
    // ` (96)
    {0x00, 0x01, 0x02, 0x04, 0x00},
    // a (97)
    {0x20, 0x54, 0x54, 0x54, 0x78},
    // b (98)
    {0x7F, 0x48, 0x44, 0x44, 0x38},
    // c (99)
    {0x38, 0x44, 0x44, 0x44, 0x20},
    // d (100)
    {0x38, 0x44, 0x44, 0x48, 0x7F},
    // e (101)
    {0x38, 0x54, 0x54, 0x54, 0x18},
    // f (102)
    {0x08, 0x7E, 0x09, 0x01, 0x02},
    // g (103)
    {0x08, 0x14, 0x54, 0x54, 0x3C},
    // h (104)
    {0x7F, 0x08, 0x04, 0x04, 0x78},
    // i (105)
    {0x00, 0x44, 0x7D, 0x40, 0x00},
    // j (106)
    {0x20, 0x40, 0x44, 0x3D, 0x00},
    // k (107)
    {0x00, 0x7F, 0x10, 0x28, 0x44},
    // l (108)
    {0x00, 0x41, 0x7F, 0x40, 0x00},
    // m (109)
    {0x7C, 0x04, 0x18, 0x04, 0x78},
    // n (110)
    {0x7C, 0x08, 0x04, 0x04, 0x78},
    // o (111)
    {0x38, 0x44, 0x44, 0x44, 0x38},
    // p (112)
    {0x7C, 0x14, 0x14, 0x14, 0x08},
    // q (113)
    {0x08, 0x14, 0x14, 0x18, 0x7C},
    // r (114)
    {0x7C, 0x08, 0x04, 0x04, 0x08},
    // s (115)
    {0x48, 0x54, 0x54, 0x54, 0x20},
    // t (116)
    {0x04, 0x3F, 0x44, 0x40, 0x20},
    // u (117)
    {0x3C, 0x40, 0x40, 0x20, 0x7C},
    // v (118)
    {0x1C, 0x20, 0x40, 0x20, 0x1C},
    // w (119)
    {0x3C, 0x40, 0x30, 0x40, 0x3C},
    // x (120)
    {0x44, 0x28, 0x10, 0x28, 0x44},
    // y (121)
    {0x0C, 0x50, 0x50, 0x50, 0x3C},
    // z (122)
    {0x44, 0x64, 0x54, 0x4C, 0x44},
    // { (123)
    {0x00, 0x08, 0x36, 0x41, 0x00},
    // | (124)
    {0x00, 0x00, 0x7F, 0x00, 0x00},
    // } (125)
    {0x00, 0x41, 0x36, 0x08, 0x00},
    // ~ (126)
    {0x08, 0x08, 0x2A, 0x1C, 0x08},
};

static void drawCharToTarget(CPURenderTarget& target, char c, int x, int y, 
                              const Color& color, float fontSize) {
    if (c < ' ' || c > '~') return;

    const uint8_t* charData = FONT_5X7[c - ' '];
    float scale = fontSize / 7.0f;  // 基于 7 行高度缩放

    for (int row = 0; row < 7; ++row) {
        uint8_t rowBits = charData[row];
        for (int col = 0; col < 5; ++col) {
            if (rowBits & (1 << (4 - col))) {
                // 该像素需要绘制
                int px = x + static_cast<int>(col * scale);
                int py = y + static_cast<int>(row * scale);
                // 根据缩放填充像素块
                int blockSize = static_cast<int>(scale + 0.5f);
                if (blockSize < 1) blockSize = 1;
                for (int dy = 0; dy < blockSize; ++dy) {
                    for (int dx = 0; dx < blockSize; ++dx) {
                        target.setPixelSafe(px + dx, py + dy, color);
                    }
                }
            }
        }
    }
}

void ToolText::renderOverlay(CPURenderTarget& overlay, const RenderContext& ctx) {
    if (!editing_) return;

    // 更新光标闪烁计时
    cursorTimer_ += ctx.deltaTime;
    if (cursorTimer_ >= 0.5f) {
        cursorTimer_ = 0.0f;
        cursorVisible_ = !cursorVisible_;
    }

    // 绘制文本框边框
    Color border(0.3f, 0.6f, 1.0f, 0.8f);
    int x = static_cast<int>(textPos_.x);
    int y = static_cast<int>(textPos_.y);

    // 计算文本框尺寸（支持多行）
    int lineCount = 1;
    int maxLineLength = 0;
    int currentLineLength = 0;
    for (char c : text_) {
        if (c == '\n') {
            lineCount++;
            maxLineLength = std::max(maxLineLength, currentLineLength);
            currentLineLength = 0;
        } else {
            currentLineLength++;
        }
    }
    maxLineLength = std::max(maxLineLength, currentLineLength);
    if (maxLineLength == 0) maxLineLength = 1;

    int w = static_cast<int>(maxLineLength * fontSize_ * 0.6f) + 10;
    int h = static_cast<int>(lineCount * fontSize_) + 10;

    // 绘制边框
    for (int i = 0; i < w; ++i) {
        overlay.setPixelSafe(x + i, y, border);
        overlay.setPixelSafe(x + i, y + h, border);
    }
    for (int i = 0; i < h; ++i) {
        overlay.setPixelSafe(x, y + i, border);
        overlay.setPixelSafe(x + w, y + i, border);
    }

    // 绘制已输入文本（简化点阵）
    int tx = x + 5;
    int ty = y + 5;
    int cursorX = tx;
    int cursorY = ty;

    for (char c : text_) {
        if (c == '\n') {
            // 换行
            cursorX = tx;
            cursorY += static_cast<int>(fontSize_);
            continue;
        }

        // 绘制字符到 overlay
        drawCharToTarget(overlay, c, cursorX, cursorY, textColor_, fontSize_);
        cursorX += static_cast<int>(fontSize_ * 0.6f);
    }

    // 绘制闪烁光标
    if (cursorVisible_) {
        Color cursorColor(0.3f, 0.6f, 1.0f, 0.9f);
        int cw = 2;  // 光标宽度
        int ch = static_cast<int>(fontSize_);
        for (int dy = 0; dy < ch; ++dy) {
            for (int dx = 0; dx < cw; ++dx) {
                overlay.setPixelSafe(cursorX + dx, cursorY + dy, cursorColor);
            }
        }
    }
}