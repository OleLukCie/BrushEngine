#include "shortcut.h"
#include <windows.h>
#include <sstream>
#include <unordered_map>
#include <algorithm>

// ========== ShortcutKey ==========

std::string ShortcutKey::toString() const {
    return ShortcutUtils::format(*this);
}

bool ShortcutKey::operator==(const ShortcutKey& other) const {
    return normalKey == other.normalKey && 
           funcKey == other.funcKey && 
           modifiers == other.modifiers;
}

bool ShortcutKey::operator<(const ShortcutKey& other) const {
    if (modifiers != other.modifiers) 
        return static_cast<uint32_t>(modifiers) < static_cast<uint32_t>(other.modifiers);
    if (funcKey != other.funcKey)
        return static_cast<int>(funcKey) < static_cast<int>(other.funcKey);
    return normalKey < other.normalKey;
}

// ========== ShortcutUtils ==========

std::string ShortcutUtils::format(const ShortcutKey& key) {
    std::string result;
    
    auto mod = key.modifiers;
    if (hasModifier(mod, ModifierKey::Ctrl))  result += "Ctrl+";
    if (hasModifier(mod, ModifierKey::Shift)) result += "Shift+";
    if (hasModifier(mod, ModifierKey::Alt))   result += "Alt+";
    if (hasModifier(mod, ModifierKey::Win))   result += "Win+";
    
    if (key.normalKey != 0) {
        result += key.normalKey;
    } else if (key.funcKey != FunctionKey::None) {
        static const std::unordered_map<FunctionKey, std::string> funcNames = {
            {FunctionKey::F1, "F1"}, {FunctionKey::F2, "F2"},
            {FunctionKey::F3, "F3"}, {FunctionKey::F4, "F4"},
            {FunctionKey::F5, "F5"}, {FunctionKey::F6, "F6"},
            {FunctionKey::F7, "F7"}, {FunctionKey::F8, "F8"},
            {FunctionKey::F9, "F9"}, {FunctionKey::F10, "F10"},
            {FunctionKey::F11, "F11"}, {FunctionKey::F12, "F12"},
            {FunctionKey::Escape, "Escape"}, {FunctionKey::Tab, "Tab"},
            {FunctionKey::Space, "Space"}, {FunctionKey::Enter, "Enter"},
            {FunctionKey::Delete, "Delete"}, {FunctionKey::Backspace, "Backspace"},
            {FunctionKey::Up, "Up"}, {FunctionKey::Down, "Down"},
            {FunctionKey::Left, "Left"}, {FunctionKey::Right, "Right"},
        };
        auto it = funcNames.find(key.funcKey);
        if (it != funcNames.end()) result += it->second;
    }
    
    // 去掉末尾的+
    if (!result.empty() && result.back() == '+') result.pop_back();
    return result;
}

ShortcutKey ShortcutUtils::parse(const std::string& shortcutStr) {
    ShortcutKey result;
    std::string token;
    std::istringstream iss(shortcutStr);
    
    while (std::getline(iss, token, '+')) {
        // 去除空格
        token.erase(std::remove_if(token.begin(), token.end(), ::isspace), token.end());
        
        if (token == "Ctrl") result.modifiers = result.modifiers | ModifierKey::Ctrl;
        else if (token == "Shift") result.modifiers = result.modifiers | ModifierKey::Shift;
        else if (token == "Alt") result.modifiers = result.modifiers | ModifierKey::Alt;
        else if (token == "Win") result.modifiers = result.modifiers | ModifierKey::Win;
        else if (token.length() == 1) result.normalKey = token[0];
        else if (token == "F1") result.funcKey = FunctionKey::F1;
        else if (token == "F2") result.funcKey = FunctionKey::F2;
        else if (token == "F3") result.funcKey = FunctionKey::F3;
        else if (token == "F4") result.funcKey = FunctionKey::F4;
        else if (token == "F5") result.funcKey = FunctionKey::F5;
        else if (token == "F6") result.funcKey = FunctionKey::F6;
        else if (token == "F7") result.funcKey = FunctionKey::F7;
        else if (token == "F8") result.funcKey = FunctionKey::F8;
        else if (token == "F9") result.funcKey = FunctionKey::F9;
        else if (token == "F10") result.funcKey = FunctionKey::F10;
        else if (token == "F11") result.funcKey = FunctionKey::F11;
        else if (token == "F12") result.funcKey = FunctionKey::F12;
        else if (token == "Delete") result.funcKey = FunctionKey::Delete;
        else if (token == "Space") result.funcKey = FunctionKey::Space;
    }
    
    return result;
}

ShortcutKey ShortcutUtils::fromWinVK(uint32_t vkCode, bool ctrl, bool shift, bool alt) {
    ShortcutKey key;
    
    if (ctrl)  key.modifiers = key.modifiers | ModifierKey::Ctrl;
    if (shift) key.modifiers = key.modifiers | ModifierKey::Shift;
    if (alt)   key.modifiers = key.modifiers | ModifierKey::Alt;
    
    // 字母 A-Z
    if (vkCode >= 'A' && vkCode <= 'Z') {
        key.normalKey = static_cast<char>(vkCode);
    }
    // 数字 0-9
    else if (vkCode >= '0' && vkCode <= '9') {
        key.normalKey = static_cast<char>(vkCode);
    }
    // 功能键
    else if (vkCode >= VK_F1 && vkCode <= VK_F12) {
        key.funcKey = static_cast<FunctionKey>(static_cast<int>(FunctionKey::F1) + (vkCode - VK_F1));
    }
    else {
        switch (vkCode) {
            case VK_SPACE:  key.funcKey = FunctionKey::Space; break;
            case VK_DELETE: key.funcKey = FunctionKey::Delete; break;
            case VK_BACK:   key.funcKey = FunctionKey::Backspace; break;
            case VK_RETURN: key.funcKey = FunctionKey::Enter; break;
            case VK_ESCAPE: key.funcKey = FunctionKey::Escape; break;
            case VK_TAB:    key.funcKey = FunctionKey::Tab; break;
            case VK_UP:     key.funcKey = FunctionKey::Up; break;
            case VK_DOWN:   key.funcKey = FunctionKey::Down; break;
            case VK_LEFT:   key.funcKey = FunctionKey::Left; break;
            case VK_RIGHT:  key.funcKey = FunctionKey::Right; break;
            case 191:       key.normalKey = '/'; break; // VK_OEM_2
            case 219:       key.normalKey = '['; break; // VK_OEM_4
            case 221:       key.normalKey = ']'; break; // VK_OEM_6
            case 186:       key.normalKey = ';'; break; // VK_OEM_1
            case 192:       key.normalKey = '`'; break; // VK_OEM_3
        }
    }
    
    return key;
}

// ========== ShortcutManager ==========

ShortcutManager::ShortcutManager() = default;

uint32_t ShortcutManager::keyHash(const ShortcutKey& key) const {
    return (static_cast<uint32_t>(key.modifiers) << 16) | 
           (static_cast<uint32_t>(key.funcKey) << 8) | 
           static_cast<uint8_t>(key.normalKey);
}

void ShortcutManager::initDefaults() {
    bindings_.clear();
    
    // ===== 文件 =====
    bind({'N', ModifierKey::Ctrl}, ShortcutCommand::FileNew, "新建");
    bind({'O', ModifierKey::Ctrl}, ShortcutCommand::FileOpen, "打开");
    bind({'S', ModifierKey::Ctrl}, ShortcutCommand::FileSave, "保存");
    bind({'S', ModifierKey::Ctrl | ModifierKey::Shift}, ShortcutCommand::FileSaveAs, "另存为");
    bind({'W', ModifierKey::Ctrl}, ShortcutCommand::FileCloseCurrent, "关闭当前");
    bind({'W', ModifierKey::Ctrl | ModifierKey::Shift}, ShortcutCommand::FileCloseAll, "关闭全部");
    bind({'P', ModifierKey::Ctrl}, ShortcutCommand::FilePrint, "打印");
    bind({'Q', ModifierKey::Ctrl}, ShortcutCommand::FileExit, "退出");
    
    // ===== 编辑 =====
    bind({'Z', ModifierKey::Ctrl}, ShortcutCommand::EditUndo, "撤销");
    bind({'Y', ModifierKey::Ctrl}, ShortcutCommand::EditRedo, "重做");
    bind({'X', ModifierKey::Ctrl}, ShortcutCommand::EditCut, "剪切");
    bind({'C', ModifierKey::Ctrl}, ShortcutCommand::EditCopy, "复制");
    bind({'V', ModifierKey::Ctrl}, ShortcutCommand::EditPaste, "粘贴");
    bind({'T', ModifierKey::Ctrl}, ShortcutCommand::EditFreeTransform, "自由变换");
    bind({'T', ModifierKey::Ctrl | ModifierKey::Shift | ModifierKey::Alt}, 
         ShortcutCommand::EditRepeatTransform, "重复变换");
    bind({'A', ModifierKey::Ctrl}, ShortcutCommand::EditSelectAll, "全选");
    bind({'D', ModifierKey::Ctrl}, ShortcutCommand::EditDeselect, "取消选区");
    bind({'I', ModifierKey::Ctrl | ModifierKey::Shift}, ShortcutCommand::EditInvertSelection, "反选选区");
    bind(ShortcutKey(FunctionKey::Delete, ModifierKey::Alt), ShortcutCommand::EditFillForeground, "填充前景色");
    bind(ShortcutKey(FunctionKey::Delete, ModifierKey::Ctrl), ShortcutCommand::EditFillBackground, "填充背景色");
    bind(ShortcutKey(FunctionKey::Delete, ModifierKey::None), ShortcutCommand::EditDelete, "删除内容");
    bind({'K', ModifierKey::Ctrl}, ShortcutCommand::EditPreferences, "首选项");
    
    // ===== 工具 =====
    bind({'B', ModifierKey::None}, ShortcutCommand::ToolBrush, "画笔");
    bind({'V', ModifierKey::None}, ShortcutCommand::ToolMove, "移动");
    bind({'E', ModifierKey::None}, ShortcutCommand::ToolEraser, "橡皮擦");
    bind({'I', ModifierKey::None}, ShortcutCommand::ToolEyedropper, "吸管");
    bind({'Z', ModifierKey::None}, ShortcutCommand::ToolZoom, "缩放");
    bind({'R', ModifierKey::None}, ShortcutCommand::ToolRotateCanvas, "旋转画布");
    bind({'M', ModifierKey::None}, ShortcutCommand::ToolRectSelect, "矩形选区");
    bind({'M', ModifierKey::Shift}, ShortcutCommand::ToolEllipseSelect, "椭圆选区");
    bind({'L', ModifierKey::None}, ShortcutCommand::ToolLasso, "套索");
    bind({'L', ModifierKey::Shift}, ShortcutCommand::ToolPolyLasso, "多边形套索");
    bind({'W', ModifierKey::None}, ShortcutCommand::ToolMagicWand, "魔棒");
    bind({'C', ModifierKey::None}, ShortcutCommand::ToolCrop, "裁剪");
    bind({'P', ModifierKey::None}, ShortcutCommand::ToolPen, "钢笔");
    bind({'T', ModifierKey::None}, ShortcutCommand::ToolText, "文字");
    bind({'U', ModifierKey::None}, ShortcutCommand::ToolShape, "形状");
    bind({'O', ModifierKey::None}, ShortcutCommand::ToolBurn, "加深");
    bind({'O', ModifierKey::Shift}, ShortcutCommand::ToolDodge, "减淡");
    bind({'S', ModifierKey::None}, ShortcutCommand::ToolCloneStamp, "图章");
    
    // ===== 颜色 =====
    bind({'X', ModifierKey::None}, ShortcutCommand::ColorSwapForegroundBackground, "切换前后景色");
    bind({'D', ModifierKey::None}, ShortcutCommand::ColorResetDefault, "恢复默认颜色");
    
    // ===== 笔刷调节 =====
    bind({']', ModifierKey::None}, ShortcutCommand::BrushIncreaseSize, "放大笔刷", true);
    bind({'[', ModifierKey::None}, ShortcutCommand::BrushDecreaseSize, "缩小笔刷", true);
    bind({']', ModifierKey::Shift}, ShortcutCommand::BrushIncreaseHardness, "加大硬度", true);
    bind({'[', ModifierKey::Shift}, ShortcutCommand::BrushDecreaseHardness, "减小硬度", true);
    
    // 笔刷不透明度 1~0
    bind({'1', ModifierKey::None}, ShortcutCommand::BrushOpacity1, "不透明度10%");
    bind({'2', ModifierKey::None}, ShortcutCommand::BrushOpacity2, "不透明度20%");
    bind({'3', ModifierKey::None}, ShortcutCommand::BrushOpacity3, "不透明度30%");
    bind({'4', ModifierKey::None}, ShortcutCommand::BrushOpacity4, "不透明度40%");
    bind({'5', ModifierKey::None}, ShortcutCommand::BrushOpacity5, "不透明度50%");
    bind({'6', ModifierKey::None}, ShortcutCommand::BrushOpacity6, "不透明度60%");
    bind({'7', ModifierKey::None}, ShortcutCommand::BrushOpacity7, "不透明度70%");
    bind({'8', ModifierKey::None}, ShortcutCommand::BrushOpacity8, "不透明度80%");
    bind({'9', ModifierKey::None}, ShortcutCommand::BrushOpacity9, "不透明度90%");
    bind({'0', ModifierKey::None}, ShortcutCommand::BrushOpacity0, "不透明度100%");
    
    // ===== 图层 =====
    bind({'N', ModifierKey::Ctrl | ModifierKey::Shift}, ShortcutCommand::LayerNew, "新建图层");
    bind({'J', ModifierKey::Ctrl}, ShortcutCommand::LayerDuplicate, "复制图层");
    bind({'G', ModifierKey::Ctrl}, ShortcutCommand::LayerGroup, "图层编组");
    bind({'G', ModifierKey::Ctrl | ModifierKey::Shift}, ShortcutCommand::LayerUngroup, "取消编组");
    bind({'E', ModifierKey::Ctrl}, ShortcutCommand::LayerMergeDown, "向下合并");
    bind({'E', ModifierKey::Ctrl | ModifierKey::Shift}, ShortcutCommand::LayerMergeVisible, "合并可见层");
    bind({'J', ModifierKey::Ctrl | ModifierKey::Shift}, ShortcutCommand::LayerCutToNew, "剪切选区建新层");
    bind({'/', ModifierKey::None}, ShortcutCommand::LayerLockTransparent, "锁定透明像素");
    
    // ===== 视图 =====
    bind({'R', ModifierKey::Ctrl}, ShortcutCommand::ViewShowRulers, "显示标尺");
    bind({';', ModifierKey::Ctrl}, ShortcutCommand::ViewShowGuides, "显示辅助线");
    bind({';', ModifierKey::Ctrl | ModifierKey::Shift}, ShortcutCommand::ViewSnapGuides, "对齐辅助线");
    bind({'\'', ModifierKey::Ctrl}, ShortcutCommand::ViewShowGrid, "显示网格");
    bind({'1', ModifierKey::Ctrl}, ShortcutCommand::ViewActualPixels, "实际像素");
    bind({'0', ModifierKey::Ctrl}, ShortcutCommand::ViewFitCanvas, "适配画布");
    bind({'F', ModifierKey::Shift}, ShortcutCommand::ViewFlipHorizontal, "水平翻转画布");
    bind({'H', ModifierKey::None}, ShortcutCommand::ViewHideUI, "隐藏UI");
    bind(ShortcutKey{FunctionKey::F11}, ShortcutCommand::ViewFullscreen, "全屏作画");
    bind(ShortcutKey{FunctionKey::F12}, ShortcutCommand::ViewReset, "重置视图");
    
    // 临时操作（特殊处理，不在常规绑定中）
    // Space+拖动 -> ViewPanTemp (在输入层处理)
    // Alt -> ViewEyedropperTemp
    // Ctrl -> ViewMoveLayerTemp
}

void ShortcutManager::bind(const ShortcutKey& key, ShortcutCommand cmd, 
                           const std::string& desc, bool repeatable) {
    // 移除已存在的相同按键绑定
    unbind(key);
    
    ShortcutBinding binding;
    binding.key = key;
    binding.command = cmd;
    binding.description = desc;
    binding.repeatable = repeatable;
    bindings_.push_back(binding);
}

void ShortcutManager::unbind(const ShortcutKey& key) {
    bindings_.erase(
        std::remove_if(bindings_.begin(), bindings_.end(),
            [&](const ShortcutBinding& b) { return b.key == key; }),
        bindings_.end());
}

void ShortcutManager::unbindCommand(ShortcutCommand cmd) {
    bindings_.erase(
        std::remove_if(bindings_.begin(), bindings_.end(),
            [&](const ShortcutBinding& b) { return b.command == cmd; }),
        bindings_.end());
}

bool ShortcutManager::hasBinding(const ShortcutKey& key) const {
    for (const auto& b : bindings_) {
        if (b.key == key && b.enabled) return true;
    }
    return false;
}

ShortcutCommand ShortcutManager::queryCommand(const ShortcutKey& key) const {
    for (const auto& b : bindings_) {
        if (b.key == key && b.enabled) return b.command;
    }
    return ShortcutCommand::None;
}

ShortcutKey ShortcutManager::queryKey(ShortcutCommand cmd) const {
    for (const auto& b : bindings_) {
        if (b.command == cmd && b.enabled) return b.key;
    }
    return ShortcutKey();
}

void ShortcutManager::registerHandler(ShortcutCommand cmd, CommandHandler handler) {
    handlers_[cmd].push_back(handler);
}

void ShortcutManager::unregisterHandler(ShortcutCommand cmd) {
    handlers_.erase(cmd);
}

bool ShortcutManager::onKeyEvent(const ShortcutKey& key, bool pressed, bool repeat) {
    if (!globalEnabled_) return false;
    
    uint32_t hash = keyHash(key);
    
    if (pressed) {
        pressedKeys_.insert(hash);
        
        // 查找绑定
        for (const auto& binding : bindings_) {
            if (binding.key == key && binding.enabled) {
                // 非重复键，且是重复触发，跳过
                if (!binding.repeatable && repeat) continue;
                
                // 执行处理器
                auto it = handlers_.find(binding.command);
                if (it != handlers_.end()) {
                    for (auto& handler : it->second) {
                        handler(binding.command);
                    }
                }
                return true;
            }
        }
    } else {
        pressedKeys_.erase(hash);
    }
    
    return false;
}

std::vector<CommandInfo> ShortcutManager::getAllCommands() {
    return {
        // 文件
        {ShortcutCommand::FileNew, "FileNew", "新建", "文件"},
        {ShortcutCommand::FileOpen, "FileOpen", "打开", "文件"},
        {ShortcutCommand::FileSave, "FileSave", "保存", "文件"},
        {ShortcutCommand::FileSaveAs, "FileSaveAs", "另存为", "文件"},
        {ShortcutCommand::FileCloseCurrent, "FileCloseCurrent", "关闭当前", "文件"},
        {ShortcutCommand::FileCloseAll, "FileCloseAll", "关闭全部", "文件"},
        {ShortcutCommand::FilePrint, "FilePrint", "打印", "文件"},
        {ShortcutCommand::FileExit, "FileExit", "退出", "文件"},
        
        // 编辑
        {ShortcutCommand::EditUndo, "EditUndo", "撤销", "编辑"},
        {ShortcutCommand::EditRedo, "EditRedo", "重做", "编辑"},
        {ShortcutCommand::EditCut, "EditCut", "剪切", "编辑"},
        {ShortcutCommand::EditCopy, "EditCopy", "复制", "编辑"},
        {ShortcutCommand::EditPaste, "EditPaste", "粘贴", "编辑"},
        {ShortcutCommand::EditFreeTransform, "EditFreeTransform", "自由变换", "编辑"},
        {ShortcutCommand::EditRepeatTransform, "EditRepeatTransform", "重复变换", "编辑"},
        {ShortcutCommand::EditSelectAll, "EditSelectAll", "全选", "编辑"},
        {ShortcutCommand::EditDeselect, "EditDeselect", "取消选区", "编辑"},
        {ShortcutCommand::EditInvertSelection, "EditInvertSelection", "反选选区", "编辑"},
        {ShortcutCommand::EditFillForeground, "EditFillForeground", "填充前景色", "编辑"},
        {ShortcutCommand::EditFillBackground, "EditFillBackground", "填充背景色", "编辑"},
        {ShortcutCommand::EditDelete, "EditDelete", "删除内容", "编辑"},
        {ShortcutCommand::EditPreferences, "EditPreferences", "首选项", "编辑"},
        
        // 工具
        {ShortcutCommand::ToolBrush, "ToolBrush", "画笔", "工具"},
        {ShortcutCommand::ToolMove, "ToolMove", "移动", "工具"},
        {ShortcutCommand::ToolEraser, "ToolEraser", "橡皮擦", "工具"},
        {ShortcutCommand::ToolEyedropper, "ToolEyedropper", "吸管", "工具"},
        {ShortcutCommand::ToolZoom, "ToolZoom", "缩放", "工具"},
        {ShortcutCommand::ToolRotateCanvas, "ToolRotateCanvas", "旋转画布", "工具"},
        {ShortcutCommand::ToolRectSelect, "ToolRectSelect", "矩形选区", "工具"},
        {ShortcutCommand::ToolEllipseSelect, "ToolEllipseSelect", "椭圆选区", "工具"},
        {ShortcutCommand::ToolLasso, "ToolLasso", "套索", "工具"},
        {ShortcutCommand::ToolPolyLasso, "ToolPolyLasso", "多边形套索", "工具"},
        {ShortcutCommand::ToolMagicWand, "ToolMagicWand", "魔棒", "工具"},
        {ShortcutCommand::ToolCrop, "ToolCrop", "裁剪", "工具"},
        {ShortcutCommand::ToolPen, "ToolPen", "钢笔", "工具"},
        {ShortcutCommand::ToolText, "ToolText", "文字", "工具"},
        {ShortcutCommand::ToolShape, "ToolShape", "形状", "工具"},
        {ShortcutCommand::ToolBurn, "ToolBurn", "加深", "工具"},
        {ShortcutCommand::ToolDodge, "ToolDodge", "减淡", "工具"},
        {ShortcutCommand::ToolCloneStamp, "ToolCloneStamp", "图章", "工具"},
        
        // 颜色
        {ShortcutCommand::ColorSwapForegroundBackground, "ColorSwap", "切换前后景色", "颜色"},
        {ShortcutCommand::ColorResetDefault, "ColorReset", "恢复默认颜色", "颜色"},
        
        // 笔刷
        {ShortcutCommand::BrushIncreaseSize, "BrushIncSize", "放大笔刷", "笔刷"},
        {ShortcutCommand::BrushDecreaseSize, "BrushDecSize", "缩小笔刷", "笔刷"},
        {ShortcutCommand::BrushIncreaseHardness, "BrushIncHard", "加大硬度", "笔刷"},
        {ShortcutCommand::BrushDecreaseHardness, "BrushDecHard", "减小硬度", "笔刷"},
        {ShortcutCommand::BrushOpacity0, "BrushOp0", "不透明度100%", "笔刷"},
        {ShortcutCommand::BrushOpacity1, "BrushOp1", "不透明度10%", "笔刷"},
        {ShortcutCommand::BrushOpacity2, "BrushOp2", "不透明度20%", "笔刷"},
        {ShortcutCommand::BrushOpacity3, "BrushOp3", "不透明度30%", "笔刷"},
        {ShortcutCommand::BrushOpacity4, "BrushOp4", "不透明度40%", "笔刷"},
        {ShortcutCommand::BrushOpacity5, "BrushOp5", "不透明度50%", "笔刷"},
        {ShortcutCommand::BrushOpacity6, "BrushOp6", "不透明度60%", "笔刷"},
        {ShortcutCommand::BrushOpacity7, "BrushOp7", "不透明度70%", "笔刷"},
        {ShortcutCommand::BrushOpacity8, "BrushOp8", "不透明度80%", "笔刷"},
        {ShortcutCommand::BrushOpacity9, "BrushOp9", "不透明度90%", "笔刷"},
        
        // 图层
        {ShortcutCommand::LayerNew, "LayerNew", "新建图层", "图层"},
        {ShortcutCommand::LayerDuplicate, "LayerDup", "复制图层", "图层"},
        {ShortcutCommand::LayerGroup, "LayerGroup", "图层编组", "图层"},
        {ShortcutCommand::LayerUngroup, "LayerUngroup", "取消编组", "图层"},
        {ShortcutCommand::LayerMergeDown, "LayerMergeDown", "向下合并", "图层"},
        {ShortcutCommand::LayerMergeVisible, "LayerMergeVis", "合并可见层", "图层"},
        {ShortcutCommand::LayerCutToNew, "LayerCutNew", "剪切选区建新层", "图层"},
        {ShortcutCommand::LayerLockTransparent, "LayerLockTrans", "锁定透明像素", "图层"},
        
        // 视图
        {ShortcutCommand::ViewShowRulers, "ViewRulers", "显示标尺", "视图"},
        {ShortcutCommand::ViewShowGuides, "ViewGuides", "显示辅助线", "视图"},
        {ShortcutCommand::ViewSnapGuides, "ViewSnap", "对齐辅助线", "视图"},
        {ShortcutCommand::ViewShowGrid, "ViewGrid", "显示网格", "视图"},
        {ShortcutCommand::ViewActualPixels, "ViewActual", "实际像素", "视图"},
        {ShortcutCommand::ViewFitCanvas, "ViewFit", "适配画布", "视图"},
        {ShortcutCommand::ViewFlipHorizontal, "ViewFlipH", "水平翻转画布", "视图"},
        {ShortcutCommand::ViewPanTemp, "ViewPanTemp", "临时平移画布", "视图"},
        {ShortcutCommand::ViewEyedropperTemp, "ViewEyeTemp", "临时吸色", "视图"},
        {ShortcutCommand::ViewMoveLayerTemp, "ViewMoveTemp", "临时移动图层", "视图"},
        {ShortcutCommand::ViewHideUI, "ViewHideUI", "隐藏UI", "视图"},
        {ShortcutCommand::ViewFullscreen, "ViewFullscreen", "全屏作画", "视图"},
        {ShortcutCommand::ViewReset, "ViewReset", "重置视图", "视图"},
    };
}

std::string ShortcutManager::getCommandDisplayName(ShortcutCommand cmd) const {
    auto commands = getAllCommands();
    for (const auto& c : commands) {
        if (c.command == cmd) return c.displayName;
    }
    return "Unknown";
}

std::string ShortcutManager::exportConfig() const {
    // 简化的文本格式：每行 "快捷键=命令名"
    std::string result;
    for (const auto& b : bindings_) {
        result += b.key.toString() + "=" + getCommandDisplayName(b.command) + "\n";
    }
    return result;
}

void ShortcutManager::importConfig(const std::string& config) {
    // 解析并覆盖绑定
    std::istringstream iss(config);
    std::string line;
    while (std::getline(iss, line)) {
        auto pos = line.find('=');
        if (pos == std::string::npos) continue;
        
        std::string keyStr = line.substr(0, pos);
        std::string cmdName = line.substr(pos + 1);
        
        ShortcutKey key = ShortcutUtils::parse(keyStr);
        // 查找命令
        auto commands = getAllCommands();
        for (const auto& c : commands) {
            if (c.name == cmdName || c.displayName == cmdName) {
                bind(key, c.command, c.displayName);
                break;
            }
        }
    }
}