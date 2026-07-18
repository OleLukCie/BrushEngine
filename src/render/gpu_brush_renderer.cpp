#define NOMINMAX
#include "gpu_brush_renderer.h"
#include "vk_utils.h"
#include "vk_buffer.h"
#include "../base/base_math.h"
#include <cmath>
#include "base/log.h"

// ============================================================================
// 外部全局变量
// ============================================================================
extern VkDevice g_device;
extern VkPhysicalDevice g_physicalDevice;

// ============================================================================
// 生命周期
// ============================================================================

GpuBrushRenderer::GpuBrushRenderer() = default;

GpuBrushRenderer::~GpuBrushRenderer() {
    cleanup();
}

GpuBrushRenderer::GpuBrushRenderer(GpuBrushRenderer&& other) noexcept {
    *this = std::move(other);
}

GpuBrushRenderer& GpuBrushRenderer::operator=(GpuBrushRenderer&& other) noexcept {
    if (this != &other) {
        cleanup();
        settings_ = other.settings_;
        pipelineState_ = other.pipelineState_;
        pipelineInitialized_ = other.pipelineInitialized_;
        targetImage_ = other.targetImage_;
        targetView_ = other.targetView_;
        targetWidth_ = other.targetWidth_;
        targetHeight_ = other.targetHeight_;
        brushTexture_ = other.brushTexture_;
        hasBrushTexture_ = other.hasBrushTexture_;
        pendingDabs_ = std::move(other.pendingDabs_);

        other.pipelineState_ = {};
        other.pipelineInitialized_ = false;
        other.targetImage_ = VK_NULL_HANDLE;
        other.targetView_ = VK_NULL_HANDLE;
        other.targetWidth_ = 0;
        other.targetHeight_ = 0;
        other.brushTexture_ = {};
        other.hasBrushTexture_ = false;
    }
    return *this;
}

// ============================================================================
// 初始化与清理
// ============================================================================

bool GpuBrushRenderer::init() {
    cleanup();

    if (!createBrushPipeline(pipelineState_, VK_NULL_HANDLE)) {
        BE_LOG_ERROR("GpuBrushRenderer::init: failed to create brush pipeline");
        return false;
    }
    pipelineInitialized_ = true;

    return true;
}

void GpuBrushRenderer::cleanup() {
    clearPendingDabs();

    if (hasBrushTexture_) {
        destroyBrushTexture(brushTexture_);
        hasBrushTexture_ = false;
    }

    if (pipelineInitialized_) {
        destroyBrushPipeline(pipelineState_);
        pipelineInitialized_ = false;
    }

    targetImage_ = VK_NULL_HANDLE;
    targetView_ = VK_NULL_HANDLE;
    targetWidth_ = 0;
    targetHeight_ = 0;
}

// ============================================================================
// 渲染目标
// ============================================================================

void GpuBrushRenderer::setTarget(VkImage image, VkImageView view, uint32_t width, uint32_t height) {
    targetImage_ = image;
    targetView_ = view;
    targetWidth_ = width;
    targetHeight_ = height;
}

void GpuBrushRenderer::clearTarget() {
    targetImage_ = VK_NULL_HANDLE;
    targetView_ = VK_NULL_HANDLE;
    targetWidth_ = 0;
    targetHeight_ = 0;
}

// ============================================================================
// 笔刷纹理
// ============================================================================

void GpuBrushRenderer::setBrushTexture(const Texture* texture) {
    if (hasBrushTexture_) {
        destroyBrushTexture(brushTexture_);
        hasBrushTexture_ = false;
    }

    if (texture != nullptr) {
        brushTexture_ = createBrushTexture(texture);
        hasBrushTexture_ = (brushTexture_.view != VK_NULL_HANDLE);
    }
}

void GpuBrushRenderer::clearBrushTexture() {
    if (hasBrushTexture_) {
        destroyBrushTexture(brushTexture_);
        hasBrushTexture_ = false;
    }
}

// ============================================================================
// 渲染接口
// ============================================================================

int GpuBrushRenderer::renderStroke(const Vec2& from, const Vec2& to,
                                    const BrushRenderState& fromState,
                                    const BrushRenderState& toState) {
    if (targetView_ == VK_NULL_HANDLE) {
        BE_LOG_ERROR("GpuBrushRenderer::renderStroke: no target set");
        return 0;
    }

    // 计算两点之间距离
    float dist = euclideanDistance(from.x, from.y, to.x, to.y);
    if (dist < 1e-3f) return 0;

    // 计算平均压力
    float avgPressure = (fromState.pressure + toState.pressure) * 0.5f;
    float size = computeSize(avgPressure);

    // 基础间距
    float spacing = settings_.baseSpacing;
    if (settings_.adaptiveSpacing) {
        spacing = adaptiveStepLength(settings_.baseSpacing, avgPressure);
    }

    // 计算笔刷步长距离
    float stepDist = size * 2.0f * spacing;
    int numSteps = static_cast<int>(std::ceil(dist / stepDist));
    if (numSteps < 1) numSteps = 1;

    // 检查 pendingDabs_ 容量
    if (pendingDabs_.size() + numSteps + 1 > MAX_PENDING_DABS) {
        BE_LOG_ERROR("GpuBrushRenderer::renderStroke: pending dabs overflow");
        return 0;
    }

    // 插值生成 dab
    int count = 0;
    for (int i = 0; i <= numSteps; ++i) {
        float t = static_cast<float>(i) / numSteps;

        BrushRenderState state;
        state.position = lerpVec2(from, to, t);
        state.pressure = lerp(fromState.pressure, toState.pressure, t);
        state.velocity = lerp(fromState.velocity, toState.velocity, t);
        state.direction = DirectionP(from.x, from.y, to.x, to.y);

        pendingDabs_.push_back(buildDabInstance(state));
        count++;
    }

    return count;
}

int GpuBrushRenderer::renderStrokePath(const std::vector<BrushRenderState>& states) {
    if (states.size() < 2) return 0;

    int totalDabs = 0;
    for (size_t i = 1; i < states.size(); ++i) {
        totalDabs += renderStroke(
            states[i - 1].position, states[i].position,
            states[i - 1], states[i]
        );
    }
    return totalDabs;
}

int GpuBrushRenderer::renderSmoothStroke(const std::vector<Vec2>& points,
                                          const std::vector<float>& pressures,
                                          float smoothing) {
    if (points.size() < 4 || points.size() != pressures.size()) return 0;

    std::vector<BrushRenderState> states;

    for (size_t i = 1; i < points.size() - 2; ++i) {
        const Vec2& P0 = points[i - 1];
        const Vec2& P1 = points[i];
        const Vec2& P2 = points[i + 1];
        const Vec2& P3 = points[i + 2];

        float len = arcLen(P0, P1, P2, P3, 20);
        int samples = static_cast<int>(std::ceil(len / 2.0f));
        samples = clamp(samples, 3, 50);

        for (int s = 0; s < samples; ++s) {
            float t = static_cast<float>(s) / (samples - 1);
            Vec2 pos = catmullRom(P0, P1, P2, P3, t);
            float p = lerp(pressures[i], pressures[i + 1], t);

            BrushRenderState state;
            state.position = pos;
            state.pressure = p;
            state.velocity = 0.0f;

            if (states.empty()) {
                state.direction = 0.0f;
            } else {
                state.direction = DirectionP(states.back().position.x, states.back().position.y,
                                              pos.x, pos.y);
            }

            states.push_back(state);
        }
    }

    return renderStrokePath(states);
}

// ============================================================================
// 提交
// ============================================================================

uint32_t GpuBrushRenderer::flush(VkCommandBuffer cmd) {
    if (pendingDabs_.empty()) {
        return 0;
    }

    if (!pipelineInitialized_) {
        BE_LOG_ERROR("GpuBrushRenderer::flush: pipeline not initialized");
        clearPendingDabs();
        return 0;
    }

    if (targetView_ == VK_NULL_HANDLE) {
        BE_LOG_ERROR("GpuBrushRenderer::flush: no target set");
        clearPendingDabs();
        return 0;
    }

    uint32_t rendered = recordBrushRender(
        cmd,
        pipelineState_,
        pendingDabs_,
        targetImage_,
        targetView_,
        targetWidth_,
        targetHeight_,
        hasBrushTexture_ ? &brushTexture_ : nullptr
    );

    pendingDabs_.clear();
    return rendered;
}

void GpuBrushRenderer::clearPendingDabs() {
    pendingDabs_.clear();
}

// ============================================================================
// 内部方法
// ============================================================================

DabInstance GpuBrushRenderer::buildDabInstance(const BrushRenderState& state) const {
    DabInstance dab;

    dab.color = settings_.color;
    dab.center = state.position;
    dab.size = computeSize(state.pressure);
    dab.opacity = computeOpacity(state, settings_.strokeProgress);
    dab.flattening = settings_.flattening;

    Vec2 dir(std::cos(state.direction), std::sin(state.direction));
    dab.rotation = computeRotation(dir);

    dab.textureScale = settings_.textureScale;
    dab.textureRotation = settings_.textureRotation;
    dab.useTexture = (settings_.texture != nullptr && hasBrushTexture_) ? 1u : 0u;
    dab.blendMode = static_cast<uint32_t>(settings_.blendMode);

    return dab;
}

float GpuBrushRenderer::computeSize(float pressure) const {
    return pressureToWidth(pressure, settings_.minSize, settings_.maxSize, settings_.pressureSizePower);
}

float GpuBrushRenderer::computeOpacity(const BrushRenderState& state, float strokeT) const {
    float op = settings_.opacity;

    if (settings_.pressureOpacity) {
        op *= state.pressure;
    }

    if (settings_.useVelocityOpacity) {
        op = velocityOpacity(op, state.velocity, settings_.velocityOpacityDecay);
    }

    if (settings_.useStrokeFade) {
        float fade = strokeFade(strokeT, settings_.fadeStart, settings_.fadeEnd);
        op *= fade;
    }

    return saturate(op);
}

float GpuBrushRenderer::computeRotation(const Vec2& dir) const {
    if (settings_.followDirection) {
        return Direction(dir.x, dir.y) + settings_.rotation;
    }
    return settings_.rotation;
}
