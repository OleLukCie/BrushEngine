#include "base_math.h"

// 线性插值
// lerp(a, b, t) = a + (b - a) * t
// 用于颜色渐变、笔触粗细过渡、坐标平滑移动
float lerp(float a, float b, float t) {
    // 限制插值参数t在 [0, 1] 范围内，防止越界
    t = std::max(0.0f, std::min(1.0f, t));
    return a + (b - a) * t;
}

// Vec2 版本的线性插值
Vec2 lerpVec2(const Vec2& a, const Vec2& b, float t) {
    t = std::max(0.0f, std::min(1.0f, t));
    return Vec2(
        a.x + (b.x - a.x) * t,
        a.y + (b.y - a.y) * t
    );
}

// Color 版本的线性插值（各通道分别插值）
Color lerpColor(const Color& a, const Color& b, float t) {
    t = std::max(0.0f, std::min(1.0f, t));
    return Color(
        a.r + (b.r - a.r) * t,
        a.g + (b.g - a.g) * t,
        a.b + (b.b - a.b) * t,
        a.a + (b.a - a.a) * t
    );
}



// Catmull-Rom 三次样条插值
// 传入4个控制点P0、P1、P2、P3，根据曲线参数t计算曲线上的点
// 曲线必过P1和P2，P0、P3用来控制曲线的弯曲形状
// 用于绘制顺滑笔触路径
Vec2 catmullRom(
    const Vec2& P0, // 第一个控制点，用于影响曲线的起点的切线方向
    const Vec2& P1, // 第二个控制点，曲线的起点
    const Vec2& P2, // 第三个控制点，曲线的终点
    const Vec2& P3, // 第四个控制点，用于影响曲线终点的切线方向
    float t
) {
    // 把t限制在0~1之间，防止越界
    t = std::max(0.0f, std::min(1.0f, t));

    // 计算t的平方、立方
    float t2 = t * t;
    float t3 = t2 * t;

    // 插值后的点坐标
    return Vec2(
        0.5f * (2.0f * P1.x
                + (-P0.x + P2.x) * t
                + (2.0f * P0.x - 5.0f * P1.x + 4.0f * P2.x - P3.x) * t2
                + (-P0.x + 3.0f * P1.x - 3.0f * P2.x + P3.x) * t3),

        0.5f * (2.0f * P1.y
                + (-P0.y + P2.y) * t
                + (2.0f * P0.y - 5.0f * P1.y + 4.0f * P2.y - P3.y) * t2
                + (-P0.y + 3.0f * P1.y - 3.0f * P2.y + P3.y) * t3)
    );
}


// Catmull-Rom曲线弧长公式（Simpson数值积分法）
// L = integral sqrt((dx/dt)^2 + (dy/dt)^2) dt
float arcLen(
    // Catmull-Rom 四个控制点
    const Vec2& P0,
    const Vec2& P1,
    const Vec2& P2,
    const Vec2& P3,
    int segments // 积分分段数（必须是偶数，默认100，越大越精确但越慢）
) {
    // 最少分2段，否则无法计算
    if (segments < 2) segments = 2;
    // Simpson 需要偶数分段
    if (segments % 2 == 1) segments++;

    // 每一段的步长（t的增量）
    float h = 1.0f / segments;
    // 积分求和结果
    float sum = 0.0f;

    // 计算速度模长的辅助函数
    // 速度 = 曲线导数，速度的模长 = 路径长度微分
    auto speedAt = [&](float t) -> float {
        // Catmull-Rom 曲线对 t 求导（x、y 方向导数）
        float t2 = t * t;
        // x 方向导数 dx/dt
        float dx = 0.5f * ((-P0.x + P2.x)
                   + 2.0f * (2.0f * P0.x - 5.0f * P1.x + 4.0f * P2.x - P3.x) * t
                   + 3.0f * (-P0.x + 3.0f * P1.x - 3.0f * P2.x + P3.x) * t2);
        // y 方向导数 dy/dt
        float dy = 0.5f * ((-P0.y + P2.y)
                   + 2.0f * (2.0f * P0.y - 5.0f * P1.y + 4.0f * P2.y - P3.y) * t
                   + 3.0f * (-P0.y + 3.0f * P1.y - 3.0f * P2.y + P3.y) * t2);
        // 导数向量的模长为速度大小
        return std::sqrt(dx * dx + dy * dy);
    };

    // Simpson 法则: integral ≈ h/3 * [f0 + fn + 4*(奇数点) + 2*(偶数点)]
    sum += speedAt(0.0f) + speedAt(1.0f);

    // 遍历中间所有分段
    for (int i = 1; i < segments; ++i) {
        float t = i * h;
        // 偶数段系数2，奇数段系数4
        float coeff = (i % 2 == 0) ? 2.0f : 4.0f;
        sum += coeff * speedAt(t);
    }

    // 曲线总长度
    return sum * h / 3.0f;
}


// 一阶指数平滑滤波
// p_smooth = p_prev * (1 - alpha) + p_new * alpha
float expSmoothing(float p_prev, float p_new, float alpha) {
    alpha = std::max(0.0f, std::min(1.0f, alpha));
    return p_prev * (1.0f - alpha) + p_new * alpha;
}

// Vec2 版本的指数平滑
Vec2 expSmoothingVec2(
    const Vec2& p_prev,
    const Vec2& p_new,
    float alpha
) {
    alpha = std::max(0.0f, std::min(1.0f, alpha));
    return Vec2(
        p_prev.x * (1.0f - alpha) + p_new.x * alpha,
        p_prev.y * (1.0f - alpha) + p_new.y * alpha
    );
}


// Savitzky-Golay 二阶轨迹平滑滤波
// p_smooth = (-3*p[i-2] + 12*p[i-1] + 17*p[i] + 12*p[i+1] - 3*p[i+2]) / 35
// 用多项式拟合局部点，代替简单平均
Vec2 SGSmooth(
    const std::vector<Vec2>& points, // 点序列（轨迹）
    size_t index                     // 要平滑的第几个点
) {
    size_t n = points.size();

    // 如果点总数不足5个，无法使用5点滤波，直接返回原坐标
    if (n < 5) {
        return points[index];
    }

    // 边界保护：开头2个点、结尾2个点无法取到左右各2个点，直接返回原值
    if (index < 2 || index >= n - 2) {
        return points[index];
    }

    // 取 当前点 和 前后各2个点，共5个点
    const Vec2& p_im2 = points[index - 2]; // i-2
    const Vec2& p_im1 = points[index - 1]; // i-1
    const Vec2& p_i   = points[index];     // i
    const Vec2& p_ip1 = points[index + 1]; // i+1
    const Vec2& p_ip2 = points[index + 2]; // i-1

    return Vec2(
        (-3.0f * p_im2.x + 12.0f * p_im1.x + 17.0f * p_i.x
         + 12.0f * p_ip1.x - 3.0f * p_ip2.x) / 35.0f,
        (-3.0f * p_im2.y + 12.0f * p_im1.y + 17.0f * p_i.y
         + 12.0f * p_ip1.y - 3.0f * p_ip2.y) / 35.0f
    );
}

// 对整个轨迹序列进行平滑处理
std::vector<Vec2> smoothTrajectory(
    const std::vector<Vec2>& points
) {
    std::vector<Vec2> result;
    result.reserve(points.size()); // 预分配空间，提高效率

    // 遍历每一个点，逐个平滑
    for (size_t i = 0; i < points.size(); ++i) {
        result.push_back(SGSmooth(points, i));
    }

    return result;
}