/*

| 功能      | 快捷键              |
| ------- | ---------------- |
| 新建      | Ctrl+N           |
| 打开      | Ctrl+O           |
| 保存      | Ctrl+S           |
| 另存为     | Ctrl+Shift+S     |
| 关闭当前    | Ctrl+W           |
| 关闭全部    | Ctrl+Shift+W     |
| 打印      | Ctrl+P           |
| 退出      | Ctrl+Q           |
| 撤销      | Ctrl+Z           |
| 重做      | Ctrl+Y           |
| 剪切      | Ctrl+X           |
| 复制      | Ctrl+C           |
| 粘贴      | Ctrl+V           |
| 自由变换    | Ctrl+T           |
| 重复变换    | Ctrl+Shift+Alt+T |
| 全选      | Ctrl+A           |
| 取消选区    | Ctrl+D           |
| 反选选区    | Ctrl+Shift+I     |
| 填充前景色   | Alt+Delete       |
| 填充背景色   | Ctrl+Delete      |
| 删除内容    | Delete           |
| 首选项     | Ctrl+K           |
| 画笔      | B                |
| 移动      | V                |
| 橡皮擦     | E                |
| 吸管      | I                |
| 缩放      | Z                |
| 旋转画布    | R                |
| 矩形选区    | M                |
| 椭圆选区    | Shift+M          |
| 套索      | L                |
| 多边形套索   | Shift+L          |
| 魔棒      | W                |
| 裁剪      | C                |
| 钢笔      | P                |
| 文字      | T                |
| 形状      | U                |
| 加深      | O                |
| 减淡      | Shift+O          |
| 图章      | S                |
| 切换前后景色  | X                |
| 恢复默认颜色  | D                |
| 放大笔刷    | ]                |
| 缩小笔刷    | [                |
| 加大硬度    | Shift+]          |
| 减小硬度    | Shift+[          |
| 笔刷不透明度  | 1~0              |
| 新建图层    | Ctrl+Shift+N     |
| 复制图层    | Ctrl+J           |
| 图层编组    | Ctrl+G           |
| 取消编组    | Ctrl+Shift+G     |
| 向下合并    | Ctrl+E           |
| 合并可见层   | Ctrl+Shift+E     |
| 剪切选区建新层 | Ctrl+Shift+J     |
| 锁定透明像素  | /                |
| 显示标尺    | Ctrl+R           |
| 显示辅助线   | Ctrl+;           |
| 对齐辅助线   | Ctrl+Shift+;     |
| 显示网格    | Ctrl+'           |
| 实际像素    | Ctrl+1           |
| 适配画布    | Ctrl+0           |
| 水平翻转画布  | Shift+F          |
| 临时平移画布  | 空格 + 拖动          |
| 临时吸色    | Alt              |
| 临时移动图层  | Ctrl             |
| 隐藏 UI   | H                |
| 全屏作画    | F11              |
| 重置视图    | F12              |

*/

#pragma once

#include <unordered_set>
#include <string>
#include <vector>
#include <functional>
#include <cstdint>

// 修饰键枚举（支持位运算组合）
enum class ModifierKey : uint32_t {
    None    = 0,
    Ctrl    = 1 << 0,
    Shift   = 1 << 1,
    Alt     = 1 << 2,
    Win     = 1 << 3,  // Windows键 / Command键
};

inline ModifierKey operator|(ModifierKey a, ModifierKey b) {
    return static_cast<ModifierKey>(
        static_cast<uint32_t>(a) | static_cast<uint32_t>(b));
}

inline ModifierKey operator&(ModifierKey a, ModifierKey b) {
    return static_cast<ModifierKey>(
        static_cast<uint32_t>(a) & static_cast<uint32_t>(b));
}

inline bool hasModifier(ModifierKey flags, ModifierKey check) {
    return (static_cast<uint32_t>(flags) & static_cast<uint32_t>(check)) != 0;
}

// 功能键枚举
enum class FunctionKey {
    None,
    F1, F2, F3, F4, F5, F6, F7, F8, F9, F10, F11, F12,
    Escape, Tab, CapsLock, Space, Enter, Backspace,
    Delete, Insert, Home, End, PageUp, PageDown,
    Up, Down, Left, Right,
};

// 快捷键定义结构
struct ShortcutKey {
    char normalKey = 0;              // 普通字符键（A-Z, 0-9, /, [, ] 等）
    FunctionKey funcKey = FunctionKey::None;  // 功能键
    ModifierKey modifiers = ModifierKey::None; // 修饰键组合

    ShortcutKey() = default;
    
    // 普通键构造
    ShortcutKey(char key, ModifierKey mod = ModifierKey::None)
        : normalKey(key), modifiers(mod) {}
    
    // 功能键构造
    ShortcutKey(FunctionKey fk, ModifierKey mod = ModifierKey::None)
        : funcKey(fk), modifiers(mod) {}

    // 是否有效
    bool isValid() const {
        return normalKey != 0 || funcKey != FunctionKey::None;
    }

    // 字符串表示（用于显示）
    std::string toString() const;

    // 比较运算符（用于map查找）
    bool operator==(const ShortcutKey& other) const;
    bool operator<(const ShortcutKey& other) const;
};

// 快捷命令枚举：所有可绑定的功能
enum class ShortcutCommand {
    // 文件
    FileNew, FileOpen, FileSave, FileSaveAs,
    FileCloseCurrent, FileCloseAll, FilePrint, FileExit,
    
    // 编辑
    EditUndo, EditRedo,
    EditCut, EditCopy, EditPaste,
    EditFreeTransform, EditRepeatTransform,
    EditSelectAll, EditDeselect, EditInvertSelection,
    EditFillForeground, EditFillBackground, EditDelete,
    EditPreferences,
    
    // 工具
    ToolBrush, ToolMove, ToolEraser, ToolEyedropper,
    ToolZoom, ToolRotateCanvas,
    ToolRectSelect, ToolEllipseSelect,
    ToolLasso, ToolPolyLasso, ToolMagicWand,
    ToolCrop, ToolPen, ToolText, ToolShape,
    ToolDodge, ToolBurn, ToolCloneStamp,
    
    // 颜色
    ColorSwapForegroundBackground, ColorResetDefault,
    
    // 笔刷调节
    BrushIncreaseSize, BrushDecreaseSize,
    BrushIncreaseHardness, BrushDecreaseHardness,
    BrushOpacity0, BrushOpacity1, BrushOpacity2, BrushOpacity3, BrushOpacity4,
    BrushOpacity5, BrushOpacity6, BrushOpacity7, BrushOpacity8, BrushOpacity9,
    
    // 图层
    LayerNew, LayerDuplicate, LayerGroup, LayerUngroup,
    LayerMergeDown, LayerMergeVisible, LayerCutToNew,
    LayerLockTransparent,
    
    // 视图
    ViewShowRulers, ViewShowGuides, ViewSnapGuides,
    ViewShowGrid, ViewActualPixels, ViewFitCanvas,
    ViewFlipHorizontal, ViewPanTemp, ViewEyedropperTemp,
    ViewMoveLayerTemp, ViewHideUI, ViewFullscreen, ViewReset,
    
    // 特殊
    None,
};

// 命令信息
struct CommandInfo {
    ShortcutCommand command;
    std::string name;           // 内部名称
    std::string displayName;    // 显示名称
    std::string category;       // 分类（文件/编辑/工具等）
};

// 快捷键绑定条目
struct ShortcutBinding {
    ShortcutKey key;
    ShortcutCommand command;
    std::string description;    // 功能描述
    bool enabled = true;        // 是否启用
    bool repeatable = false;    // 是否可重复触发（如按住持续生效）
};

// 快捷键管理器
class ShortcutManager {
public:
    ShortcutManager();
    ~ShortcutManager() = default;

    // 初始化默认快捷键（根据快捷键设计表）
    void initDefaults();

    // 注册/注销命令处理器
    using CommandHandler = std::function<void(ShortcutCommand)>;
    
    void registerHandler(ShortcutCommand cmd, CommandHandler handler);
    void unregisterHandler(ShortcutCommand cmd);

    // 添加快捷键绑定
    void bind(const ShortcutKey& key, ShortcutCommand cmd, 
              const std::string& desc = "", bool repeatable = false);
    
    // 移除绑定
    void unbind(const ShortcutKey& key);
    void unbindCommand(ShortcutCommand cmd);

    // 查询
    bool hasBinding(const ShortcutKey& key) const;
    ShortcutCommand queryCommand(const ShortcutKey& key) const;
    ShortcutKey queryKey(ShortcutCommand cmd) const;
    std::string getCommandDisplayName(ShortcutCommand cmd) const;

    // 处理按键事件（由系统输入层调用）
    // @param key 当前按键
    // @param mods 当前修饰键状态
    // @param pressed true=按下, false=释放
    // @param repeat 是否重复触发（长按）
    // @return true=有匹配并处理, false=未匹配
    bool onKeyEvent(const ShortcutKey& key, bool pressed, bool repeat = false);

    // 获取所有绑定
    const std::vector<ShortcutBinding>& bindings() const { return bindings_; }

    // 导出/导入配置（JSON格式）
    std::string exportConfig() const;
    void importConfig(const std::string& config);

    // 临时禁用/启用所有快捷键（如模态对话框期间）
    void setGlobalEnabled(bool enabled) { globalEnabled_ = enabled; }
    bool isGlobalEnabled() const { return globalEnabled_; }

    // 获取命令列表
    static std::vector<CommandInfo> getAllCommands();

private:
    std::vector<ShortcutBinding> bindings_;
    std::unordered_map<ShortcutCommand, std::vector<CommandHandler>> handlers_;
    bool globalEnabled_ = true;

    // 当前按下的键（用于组合键检测）
    std::unordered_set<uint32_t> pressedKeys_;

    uint32_t keyHash(const ShortcutKey& key) const;
};

// 工具函数：从平台原生键码转换
namespace ShortcutUtils {
    // Windows VK Code -> ShortcutKey
    ShortcutKey fromWinVK(uint32_t vkCode, bool ctrl, bool shift, bool alt);
    
    // 字符串解析 "Ctrl+Shift+S" -> ShortcutKey
    ShortcutKey parse(const std::string& shortcutStr);
    
    // 格式化 ShortcutKey -> "Ctrl+Shift+S"
    std::string format(const ShortcutKey& key);
}