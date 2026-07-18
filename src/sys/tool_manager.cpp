#include "tool_manager.h"
#include "brush_circle.h"
#include "brush_pencil.h"
#include "brush_watercolor.h"
#include "brush_marker.h"
#include <algorithm>

BrushManager::BrushManager() = default;

void BrushManager::registerBrush(const std::string& name, BrushFactory factory) {
    brushFactories_[name] = factory;
    // 预创建实例
    if (brushInstances_.find(name) == brushInstances_.end()) {
        brushInstances_[name] = factory();
    }
    // 如果还没有激活笔刷，自动激活第一个
    if (!currentBrush_ && !brushFactories_.empty()) {
        setActiveBrush(name);
    }
}

void BrushManager::setActiveBrush(const std::string& name) {
    auto it = brushFactories_.find(name);
    if (it == brushFactories_.end()) return;

    activeBrushName_ = name;

    // 创建或复用笔刷实例
    auto instIt = brushInstances_.find(name);
    if (instIt == brushInstances_.end()) {
        brushInstances_[name] = it->second();
        instIt = brushInstances_.find(name);
    }

    // 转移所有权到 currentBrush_
    if (instIt != brushInstances_.end()) {
        // 注意：这里需要重新创建，因为 unique_ptr 不能共享
        currentBrush_ = it->second();
    }
}

std::vector<std::string> BrushManager::availableBrushes() const {
    std::vector<std::string> names;
    for (const auto& pair : brushFactories_) {
        names.push_back(pair.first);
    }
    return names;
}

BrushSettings* BrushManager::currentBrushSettings() {
    if (currentBrush_) {
        return &currentBrush_->brushSettings();
    }
    return nullptr;
}

void BrushManager::increaseBrushSize(float delta) {
    auto* settings = currentBrushSettings();
    if (settings) {
        settings->baseSize = std::min(settings->baseSize + delta, settings->maxSize);
    }
}

void BrushManager::decreaseBrushSize(float delta) {
    auto* settings = currentBrushSettings();
    if (settings) {
        settings->baseSize = std::max(settings->baseSize - delta, settings->minSize);
    }
}

void BrushManager::increaseBrushHardness(float delta) {
    auto* settings = currentBrushSettings();
    if (settings) {
        settings->flattening = std::min(settings->flattening + delta, 1.0f);
    }
}

void BrushManager::decreaseBrushHardness(float delta) {
    auto* settings = currentBrushSettings();
    if (settings) {
        settings->flattening = std::max(settings->flattening - delta, 0.1f);
    }
}

void BrushManager::setBrushOpacity(float opacity) {
    auto* settings = currentBrushSettings();
    if (settings) {
        settings->opacity = saturate(opacity);
    }
}