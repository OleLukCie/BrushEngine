#include "base_math.h"

// 余弦笔触头尾渐隐
// fade(t) = 0.5 * (1 - cos(pi * t))
float cosineFade(float t) {
    // 把t限制在0~1之间，防止越界
    t = std::max(0.0f, std::min(1.0f, t));
    // 余弦平滑曲线
    return 0.5f * (1.0f - std::cos(PI * t));
}


// 完整的头尾渐隐：起点从 0 淡入，终点淡出到 0
float strokeFade(
    float t,          // 整条轨迹的进度（0~1）
    float fadeStart,  // 起点淡入长度比例
    float fadeEnd     // 终点淡出长度比例
) {
    t = std::max(0.0f, std::min(1.0f, t));

    if (t < fadeStart) {
        // 起点淡入：从0平滑上升到1
        // t=0 时 fade=0，t=fadeStart 时 fade=1
        return cosineFade(t / fadeStart);
    }
    else if (t > 1.0f - fadeEnd) {
        // 终点淡出：从1平滑下降到0
        // t=1-fadeEnd 时 fade=1，t=1 时 fade=0
        float localT = (t - (1.0f - fadeEnd)) / fadeEnd;
        return 1.0f - cosineFade(localT);
    } else {
        // 中间段完全饱和
        return 1.0f;
    }
}


// 压感粗细非线性映射
// width = w_min + (w_max - w_min) * pressure^n
// 将数位板压感映射为符合真实笔尖物理特性的笔宽
float pressureToWidth(
    float pressure, // 压感值 [0, 1]
    float w_min,    // 最小笔触宽度
    float w_max,    // 最大笔触宽度
    float n         // 幂指数，n > 1 实现轻细重粗
) {
    pressure = std::max(0.0f, std::min(1.0f, pressure));
    return w_min + (w_max - w_min) * std::pow(pressure, n);
}


// 运笔速度计算
// v = delta_d / delta_t
// 需对速度值做平滑处理
float Velocity(float delta_d, float delta_t) {
    if (delta_t < 1e-6f) return 0.0f;  // 防止除零
    return delta_d / delta_t; // 瞬时速度
}

float V(
    float x1, float y1, float x2, float y2, float delta_t
) {
    float dist = euclideanDistance(x1, y1, x2, y2);
    return Velocity(dist, delta_t);
}


// 速度飞白透明度
// opacity = baseOpacity * max(0, 1 - v * decayFactor)
float velocityOpacity(
    float baseOpacity, //基础透明度
    float velocity,    // 当前画笔移动速度
    float decayFactor  // 衰减系数越大，速度对透明度影响越强
) {
    // 计算速度衰减因子，速度越快因子越小，最低为0
    float factor = std::max(0.0f, 1.0f - velocity * decayFactor);

    // 最终透明度 = 基础透明度 × 速度衰减
    return baseOpacity * factor;
}


// 笔触间距压感自适应
// step_len = base_step * (1 - pressure * 0.3)
// 根据压感值自适应调整笔触采样间距
float adaptiveStepLength(
    float base_step, // 基础步长（无压感时的默认间距）
    float pressure   // 压感值：0=最轻，1=最重
) {
    // 把压感限制在0~1之间，防止非法值
    pressure = std::max(0.0f, std::min(1.0f, pressure));
    // 压力越大，步长越小
    return base_step * (1.0f - pressure * 0.3f);
}