#include "brush_renderer.h"
#include <cmath>

BrushRenderer::BrushRenderer() {}

// 设置渲染目标
void BrushRenderer::setTarget(RenderTarget* target) {
    target_ = target; // 保存渲染目标指针
}

// 根据压力计算笔刷大小
float BrushRenderer::computeSize(float pressure) const {
    return pressureToWidth(pressure, settings_.minSize, settings_.maxSize, settings_.pressureSizePower);
}

// 计算笔刷最终透明度
float BrushRenderer::computeOpacity(const BrushRenderState& state, float strokeT) const {
    float op = settings_.opacity; // 基础透明度

    // 启用压力影响透明度
    if (settings_.pressureOpacity) {
        op *= state.pressure;
    }

    // 启用速度影响透明度
    if (settings_.useVelocityOpacity) {
        op = velocityOpacity(op, state.velocity, settings_.velocityOpacityDecay);
    }

    // 启用笔触渐变淡入淡出
    if (settings_.useStrokeFade) {
        // 计算渐变系数
        float fade = strokeFade(strokeT, settings_.fadeStart, settings_.fadeEnd);
        op *= fade;
    }

    return saturate(op);
}

// 计算笔刷旋转角度
float BrushRenderer::computeRotation(const Vec2& dir) const {
    // 如果启用跟随绘制方向旋转
    if (settings_.followDirection) {
        // 方向角度 + 自定义旋转角度
        return Direction(dir.x, dir.y) + settings_.rotation;
    }
    return settings_.rotation;
}

// 渲染椭圆笔触点
void BrushRenderer::renderEllipseDab(const Vec2& center, float a, float b,
                                      float rotation, const Color& color, float opacity) {
    if (!target_) return; // 无渲染目标直接返回
    if (opacity < 1e-3f) return; // 透明度极低不渲染

    // 获取画布宽高
    int w = target_->width();
    int h = target_->height();

    // 取椭圆最大半轴作为渲染半径
    float maxRadius = std::max(a, b);
    // 计算渲染区域最小X坐标
    int minX = static_cast<int>(std::floor(center.x - maxRadius));
    // 计算渲染区域最大X坐标
    int maxX = static_cast<int>(std::ceil(center.x + maxRadius));
    // 计算渲染区域最小Y坐标
    int minY = static_cast<int>(std::floor(center.y - maxRadius));
    // 计算渲染区域最大Y坐标
    int maxY = static_cast<int>(std::ceil(center.y + maxRadius));

    // 限制渲染区域在画布范围内
    minX = clamp(minX, 0, w - 1);
    maxX = clamp(maxX, 0, w - 1);
    minY = clamp(minY, 0, h - 1);
    maxY = clamp(maxY, 0, h - 1);

    // 初始化颜色混合管道
    BlendPipeline pipeline;
    // 设置混合模式
    pipeline.mode = settings_.blendMode;
    // 混合管道透明度
    pipeline.opacity = 1.0f;

    // 遍历渲染区域所有Y像素
    for (int y = minY; y <= maxY; ++y) {
        // 遍历渲染区域所有X像素
        for (int x = minX; x <= maxX; ++x) {
            // 当前像素相对于笔刷中心的X偏移
            float dx = x - center.x;
            // 当前像素相对于笔刷中心的Y偏移
            float dy = y - center.y;
            float localX, localY;
            // 将偏移坐标反向旋转到笔刷局部坐标系
            rotate2D(dx, dy, -rotation, localX, localY);

            // 计算当前像素在椭圆内的覆盖值
            float coverage = ellipseCoverage(localX, localY, a, b);
            if (coverage < 1e-3f) continue; // 覆盖值过低跳过

            Color src = color; // 笔刷颜色
            src.a = coverage * opacity; // 计算最终Alpha值

            Color dst = target_->getPixel(x, y); // 获取画布原像素
            Color result = pipeline.apply(src, dst); // 颜色混合
            target_->setPixel(x, y, result); // 写入混合后像素
        }
    }
}


// 渲染带纹理的笔触点
void BrushRenderer::renderTexturedDab(const Vec2& center, float a, float b,
                                       float rotation, const Color& color, float opacity) {
    // 无目标或无纹理，降级为椭圆笔刷
    if (!target_ || !settings_.texture) {
        renderEllipseDab(center, a, b, rotation, color, opacity);
        return;
    }

    // 获取画布宽高
    int w = target_->width();
    int h = target_->height();

    // 取椭圆最大半轴
    float maxRadius = std::max(a, b);

    // 计算渲染区域边界
    int minX = static_cast<int>(std::floor(center.x - maxRadius));
    int maxX = static_cast<int>(std::ceil(center.x + maxRadius));
    int minY = static_cast<int>(std::floor(center.y - maxRadius));
    int maxY = static_cast<int>(std::ceil(center.y + maxRadius));

    // 限制区域在画布内
    minX = clamp(minX, 0, w - 1);
    maxX = clamp(maxX, 0, w - 1);
    minY = clamp(minY, 0, h - 1);
    maxY = clamp(maxY, 0, h - 1);

    // 获取笔刷纹理
    const Texture& tex = *settings_.texture;
    // 混合管道
    BlendPipeline pipeline;
    // 设置混合模式
    pipeline.mode = settings_.blendMode;

    // 遍历区域Y坐标
    for (int y = minY; y <= maxY; ++y) {
        // 遍历区域X坐标
        for (int x = minX; x <= maxX; ++x) {
            // 像素相对中心偏移
            float dx = x - center.x;
            float dy = y - center.y;
            float localX, localY;

            // 坐标旋转到局部空间
            rotate2D(dx, dy, -rotation, localX, localY);

            // 椭圆覆盖计算
            float coverage = ellipseCoverage(localX, localY, a, b);
            if (coverage < 1e-3f) continue;

            // 计算纹理UV坐标
            float u = (localX / a) * 0.5f + 0.5f;
            float v = (localY / b) * 0.5f + 0.5f;

            // 采样纹理
            Color texColor = tex.sampleTransformed(u, v, settings_.textureRotation,
                                                    settings_.textureScale);
            Color src = color * texColor;

            // 最终Alpha
            src.a = coverage * opacity * texColor.a;

            // 原画布像素
            Color dst = target_->getPixel(x, y);
            // 混合
            Color result = pipeline.apply(src, dst);
            // 写入像素
            target_->setPixel(x, y, result);
        }
    }
}


// 渲染单个笔触点
void BrushRenderer::renderDab(const Vec2& center, float size, float opacity,
                               float rotation, const Color& color) {
    float a, b;
    // 根据尺寸和压扁值计算椭圆长短轴
    EllipseAxes(size, settings_.flattening, a, b);

    // 有纹理则渲染纹理笔刷
    if (settings_.texture) {
        renderTexturedDab(center, a, b, rotation, color, opacity);
    } else {
        // 无纹理渲染椭圆笔刷
        renderEllipseDab(center, a, b, rotation, color, opacity);
    }
}

// 根据画笔状态渲染笔触点
void BrushRenderer::renderDab(const BrushRenderState& state) {
    // 计算笔刷大小
    float size = computeSize(state.pressure);
    
    // 计算笔刷透明度
    float opacity = computeOpacity(state, settings_.strokeProgress);

    // 通过方向角计算方向向量
    Vec2 dir(std::cos(state.direction), std::sin(state.direction));
    
    // 计算旋转角度
    float rotation = computeRotation(dir);

    // 执行笔刷渲染
    renderDab(state.position, size, opacity, rotation, settings_.color);
}

// 渲染两点之间的连续笔触
int BrushRenderer::renderStroke(const Vec2& from, const Vec2& to,
                                 const BrushRenderState& fromState, const BrushRenderState& toState) {
    if (!target_) return 0; // 无目标返回

    // 计算两点之间距离
    float dist = euclideanDistance(from.x, from.y, to.x, to.y);
    if (dist < 1e-3f) return 0; // 距离过短不渲染

    // 计算平均压力
    float avgPressure = (fromState.pressure + toState.pressure) * 0.5f;
    // 计算平均笔刷大小
    float size = computeSize(avgPressure);
    // 基础间距
    float spacing = settings_.baseSpacing;
    // 如果启用自适应间距
    if (settings_.adaptiveSpacing) {
        spacing = adaptiveStepLength(settings_.baseSpacing, avgPressure);
    }

    // 计算笔刷步长距离
    float stepDist = size * 2.0f * spacing;
    // 计算需要的笔刷点数
    int numSteps = static_cast<int>(std::ceil(dist / stepDist));
    // 至少渲染一个点
    if (numSteps < 1) numSteps = 1;

    // 笔刷点数统计
    int count = 0;
    // 遍历所有步长点
    for (int i = 0; i <= numSteps; ++i) {
        float t = static_cast<float>(i) / numSteps; // 插值系数

        // 插值计算位置
        Vec2 pos = lerpVec2(from, to, t);

        // 插值计算压力
        float pressure = lerp(fromState.pressure, toState.pressure, t);

        // 插值计算速度
        float velocity = lerp(fromState.velocity, toState.velocity, t);

        // 计算笔触方向角
        float dir = DirectionP(from.x, from.y, to.x, to.y);

        // 构造当前点画笔状态
        BrushRenderState state;
        state.position = pos;
        state.pressure = pressure;
        state.velocity = velocity;
        state.direction = dir;

        // 设置笔触进度
        settings_.strokeProgress = t;

        renderDab(state); // 渲染当前点
        count++;
    }

    return count; // 返回总笔触点数
}


// 渲染多点路径组成的完整笔触
int BrushRenderer::renderStrokePath(const std::vector<BrushRenderState>& states) {
    if (states.size() < 2) return 0; // 点数量不足返回

    int totalDabs = 0; // 总笔刷点数
    // 遍历所有连续点
    for (size_t i = 1; i < states.size(); ++i) {
        // 渲染两点之间的笔触并累加点数
        totalDabs += renderStroke(states[i - 1].position, states[i].position,
                                    states[i - 1], states[i]);
    }
    return totalDabs;
}

// 渲染平滑曲线笔触
int BrushRenderer::renderSmoothStroke(const std::vector<Vec2>& points,
                                       const std::vector<float>& pressures,
                                       float smoothing) {
    // 点数量不足或压力数组不匹配返回
    if (points.size() < 4 || points.size() != pressures.size()) return 0;

    // 保存平滑后的状态点
    std::vector<BrushRenderState> states;

    // 遍历中间点（曲线平滑需要4个点）
    for (size_t i = 1; i < points.size() - 2; ++i) {
        const Vec2& P0 = points[i - 1]; // 前控制点
        const Vec2& P1 = points[i]; // 起点
        const Vec2& P2 = points[i + 1]; // 终点
        const Vec2& P3 = points[i + 2]; // 后控制点

        // 计算曲线长度
        float len = arcLen(P0, P1, P2, P3, 20);
        // 计算采样点数
        int samples = static_cast<int>(std::ceil(len / 2.0f));
        samples = clamp(samples, 3, 50);

        // 对曲线进行采样
        for (int s = 0; s < samples; ++s) {
            float t = static_cast<float>(s) / (samples - 1); // 采样系数
            Vec2 pos = catmullRom(P0, P1, P2, P3, t); // 平滑插值位置

            // 插值压力
            float p = lerp(pressures[i], pressures[i + 1], t);

            // 构造状态
            BrushRenderState state;
            state.position = pos;
            state.pressure = p;
            state.velocity = 0.0f;

            // 计算方向
            if (states.empty()) {
                state.direction = 0.0f;
            } else {
                state.direction = DirectionP(states.back().position.x, states.back().position.y,
                                              pos.x, pos.y);
            }

            states.push_back(state); // 加入状态列表
        }
    }

    return renderStrokePath(states); // 渲染平滑笔触
}
