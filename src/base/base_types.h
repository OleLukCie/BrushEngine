#pragma once // 防止头文件被重复包含


#include <cmath> // 数学函数库
#include <algorithm> // 常用算法
#include <vector> // 动态数组

// 2D向量结构体
struct Vec2 {
	// x横坐标，y纵坐标
	float x, y;

	// 构造函数：创建向量时可以给x、y赋值，默认都是0
	// 例：	Vec2 a; -> (0,0)
	// 		Vec2 a(1,2); -> (1,2)
	Vec2(float x = 0, float y = 0) : x(x), y(y) {}

	// 向量加法
	// o = other	a+b <=> a.operator+(b)
	Vec2 operator+(const Vec2& o) const { return Vec2(x + o.x, y + o.y); }

	// 向量减法
	Vec2 operator-(const Vec2& o) const { return Vec2(x - o.x, y - o.y); }

	// 向量数乘
	Vec2 operator*(float s) const { return Vec2(x * s, y * s); }

	// 向量数除
	Vec2 operator/(float s) const { return Vec2(x / s, y / s); }

	// 点积
	float dot(const Vec2& o) const { return x * o.x + y * o.y; }

	// 向量模长
	float length() const { return std::sqrt(x * x + y * y); }

	// 向量模长的平方
	float lengthSq() const { return x * x + y * y; }

	// 归一化：把向量变成长度为1的单位向量
	Vec2 normalized() const {
		float len = length();
		if (len < 1e-6f) return Vec2(0, 0);
		return Vec2(x / len, y / len);
	}
};


// 3D向量结构体
struct Vec3 {
    float x, y, z;

    //	Vec3 a; -> (0,0,0)
    //	Vec3 a(1,2,3) -> (1,2,3)
    Vec3(float x = 0, float y = 0, float z = 0) : x(x), y(y), z(z) {}

    Vec3 operator+(const Vec3& o) const { return Vec3(x + o.x, y + o.y, z + o.z); }

    Vec3 operator-(const Vec3& o) const { return Vec3(x - o.x, y - o.y, z - o.z); }

    Vec3 operator*(float s) const { return Vec3(x * s, y * s, z * s); }

    Vec3 operator/(float s) const { return Vec3(x / s, y / s, z / s); }

    float dot(const Vec3& o) const { return x * o.x + y * o.y + z * o.z; }

    float length() const { return std::sqrt(x * x + y * y + z * z); }

    float lengthSq() const { return x * x + y * y + z * z; }

    // 归一化：把向量变成长度为1的单位向量
    Vec3 normalized() const {
    	// 获取当前向量长度
        float len = length();

        // 如果长度太小就视为零向量
        if (len < 1e-6f) return Vec3(0, 0, 0);

        // 每个分量除以长度
        return Vec3(x / len, y / len, z / len);
    }
};


// 颜色结构体
struct Color {
	// RGBA： 红色、绿色、蓝色、不透明度
    float r, g, b, a;

    // Color c; -> (0,0,0,1) 纯黑
    // Color c(1,0,0); -> (1,0,0,1) 纯红
    // Color c(1,1,1,0.5f) -> (1,1,1,0.5f) 白色半透明
    constexpr Color(float r = 0, float g = 0, float b = 0, float a = 1)
        : r(r), g(g), b(b), a(a) {}

    // 颜色加法（逐分量）
    constexpr Color operator+(const Color& o) const { return Color(r + o.r, g + o.g, b + o.b, a + o.a); }

    // 颜色减法（逐分量）
    constexpr Color operator-(const Color& o) const { return Color(r - o.r, g - o.g, b - o.b, a - o.a); }

    // 颜色数乘（缩放亮度/不透明度）
    constexpr Color operator*(float s) const { return Color(r * s, g * s, b * s, a * s); }

    // 颜色数除
    constexpr Color operator/(float s) const { return Color(r / s, g / s, b / s, a / s); }

    // 颜色逐分量乘法（正片叠底/Modulate）
    constexpr Color operator*(const Color& o) const { return Color(r * o.r, g * o.g, b * o.b, a * o.a); }
};


// 圆周率
static constexpr float PI = 3.14159265358979323846f;


// 将值限制在 [min, max] 范围内
template<typename T>
inline T clamp(T value, T minVal, T maxVal) {
    return std::max(minVal, std::min(value, maxVal));
}

// 将浮点数限制在 [0, 1] 范围内（saturate）
inline float saturate(float value) {
    return clamp(value, 0.0f, 1.0f);
}

// 将颜色各分量限制在 [0, 1] 范围内
inline Color saturateColor(const Color& c) {
    return Color(saturate(c.r), saturate(c.g), saturate(c.b), saturate(c.a));
}