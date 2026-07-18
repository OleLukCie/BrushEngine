#include "base_math.h"

// 欧式距离公式
// d = sqrt((x2-x1)^2 + (y2-y1)^2)
// 用于判断笔触采样点间距，决定是否需要在中间插值额外笔触点
float euclideanDistance(float x1, float y1, float x2, float y2) {
    float dx = x2 - x1;
    float dy = y2 - y1;
    return std::sqrt(dx * dx + dy * dy);
}


// 二维坐标旋转变换
// x' = x*cos(theta) - y*sin(theta)
// y' = x*sin(theta) + y*cos(theta)
void rotate2D(
    float x, float y,           // 原始坐标
    float theta,                // 旋转角度（弧度制）
    float& x_out, float& y_out  // 旋转后的坐标
) {
    // 预先计算余弦、正弦值，避免重复计算。
    float c = std::cos(theta);
    float s = std::sin(theta);

    // 二维旋转公式
    x_out = x * c - y * s;
    y_out = x * s + y * c;
}

// Vec2 版本，直接返回新向量
Vec2 rotate2D(const Vec2& p, float theta) {
    float c = std::cos(theta);
    float s = std::sin(theta);
    return Vec2(p.x * c - p.y * s, p.x * s + p.y * c);
}


// 椭圆笔刷包围判定
// x^2/a^2 + y^2/b^2 <= 1
// 判断点是否在椭圆内部
bool isInsideEllipse(
    float x, float y, // 待判断的点坐标
    float a, float b  // 椭圆半长轴、半短轴
) {
    // 防止无效
    if (a < 1e-6f || b < 1e-6f) return false;

    // 改写为 b^2 * x^2 + a^2 * y^2 <= a^2 * b^2，避免除法
    float lhs = (b * b) * (x * x) + (a * a) * (y * y);
    float rhs = (a * a) * (b * b);

    // 小于等于 -> 在内部
    return lhs <= rhs;
}

// 带抗锯齿的椭圆 SDF（有符号距离场）
// 计算点(x,y)对椭圆的覆盖值，返回 [0,1]
float ellipseCoverage(float x, float y, float a, float b) {
    if (a < 1e-6f || b < 1e-6f) return 0.0f;

    // 把椭圆映射成单位圆
    float dx = x / a;
    float dy = y / b;
    // 计算到圆心的距离
    float dist = std::sqrt(dx * dx + dy * dy);

    // 抗锯齿过渡
    float edgeWidth = 1.0f / std::min(a, b);

    // dist ≤ 1 → 内部 = 1
    // dist 在 1~1+edgeWidth → 边缘过渡 1→0
    // dist 更大 → 外部 = 0
    return std::max(0.0f, std::min(1.0f, 1.0f - (dist - 1.0f) / edgeWidth));
}

// 椭圆长短半轴倾斜计算
// a = radius, b = radius * (1 - flattening * 0.7)
// 用于模拟真实笔尖倾斜时从圆形变为椭圆形的现象
void EllipseAxes(
    float radius, // 基础半径（椭圆最大宽度的一半）
    float flattening, // 压扁度：0=正圆，1=最扁，由倾斜角计算
    float& a,         // 长半轴
    float& b          // 短半轴
) {
    // 把压扁度限制在[0,1]范围，防止非法值
    flattening = std::max(0.0f, std::min(1.0f, flattening));
    // 长半轴固定等于基础半径
    a = radius;
    // flattening=0 → b=radius → 正圆
    // flattening=1 → b=radius*0.3 → 最扁
    b = radius * (1.0f - flattening * 0.7f);
}


// 运笔方向旋转角
// theta = atan2(dy, dx)
// 根据方向向量计算弧度制旋转角
float Direction(float dx, float dy) {
    // atan2(dy, dx) → 返回 [-π, π] 弧度角，代表从 X 轴到向量的角度
    return std::atan2(dy, dx);
}

// 传入起点和终点，自动计算方向，返回运笔角度
float DirectionP(
    float x1, float y1, // 上一个点
    float x2, float y2  // 当前点
) {
    // 用两点差计算方向向量，再调用Direction
    return Direction(x2 - x1, y2 - y1);
}