#pragma once

#include "brush.h"
#include "shortcut.h"
#include "../render/canvas.h"
#include <memory>
#include <vector>
#include <unordered_map>
#include <functional>
#include <string>
#include "tool_type.h"

// 笔刷工厂类型
using BrushFactory = std::function<std::unique_ptr<Brush>()>;

// 笔刷管理器：管理笔刷切换和属性
class BrushManager {
public:
    BrushManager();
    ~BrushManager() = default;

    // 注册笔刷工厂
    void registerBrush(const std::string& name, BrushFactory factory);

    // 切换当前笔刷
    void setActiveBrush(const std::string& name);
    std::string activeBrushName() const { return activeBrushName_; }
    std::vector<std::string> availableBrushes() const;

    // 获取当前笔刷
    Brush* currentBrush() const { return currentBrush_.get(); }
    BrushSettings* currentBrushSettings();

    // 笔刷属性调节
    void increaseBrushSize(float delta = 1.0f);
    void decreaseBrushSize(float delta = 1.0f);
    void increaseBrushHardness(float delta = 0.05f);
    void decreaseBrushHardness(float delta = 0.05f);
    void setBrushOpacity(float opacity);

private:
    std::string activeBrushName_ = "Circle";
    std::unique_ptr<Brush> currentBrush_;
    std::unordered_map<std::string, BrushFactory> brushFactories_;
    std::unordered_map<std::string, std::unique_ptr<Brush>> brushInstances_;
};