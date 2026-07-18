#pragma once

#include "base_types.h"

float euclideanDistance(float x1, float y1, float x2, float y2); // 欧式距离公式

float lerp(float a, float b, float t); // 线性插值
Vec2 lerpVec2(const Vec2& a, const Vec2& b, float t); // Vec2 版本的线性插值
Color lerpColor(const Color& a, const Color& b, float t); // Color 版本的线性插值

float expSmoothing(float p_prev, float p_new, float alpha); // 一阶指数平滑滤波
Vec2 expSmoothingVec2( const Vec2& p_prev, const Vec2& p_new, float alpha); // Vec2 版本的指数平滑

float pressureToWidth(float pressure, float w_min, float w_max, float n); // 压感粗细非线性映射

float Velocity(float delta_d, float delta_t); // 运笔速度计算
float V(float x1, float y1, float x2, float y2, float delta_t);

float velocityOpacity(float baseOpacity, float velocity, float decayFactor); // 速度飞白透明度

Vec2 catmullRom(const Vec2& P0, const Vec2& P1, const Vec2& P2, const Vec2& P3, float t); // Catmull-Rom 三次样条插值
float arcLen(const Vec2& P0, const Vec2& P1, const Vec2& P2, const Vec2& P3, int segments = 100);  // Catmull-Rom曲线弧长公式

Vec2 SGSmooth(const std::vector<Vec2>& points, size_t index); // Savitzky-Golay 二阶轨迹平滑滤波

bool isInsideEllipse(float x, float y, float a, float b); // 椭圆笔刷包围判定
float ellipseCoverage(float x, float y, float a, float b); // 带抗锯齿的椭圆SDF
void EllipseAxes(float radius, float flattening, float& a, float& b); // 椭圆长短半轴倾斜计算

float adaptiveStepLength(float base_step, float pressure); // 笔触间距压感自适应

float Direction(float dx, float dy); // 运笔方向旋转角
float DirectionP(float x1, float y1, float x2, float y2);

void rotate2D(float x, float y, float theta, float& x_out, float& y_out); // 二维坐标旋转变换
Vec2 rotate2D(const Vec2& p, float theta);


float cosineFade(float t); // 余弦笔触头尾渐隐
float strokeFade(float t, float fadeStart, float fadeEnd); // 完整的头尾渐隐


float gaussian2D(float x, float y, float sigma); // 二维高斯模糊核
void gaussianBlur(const std::vector<float>& image, int width, int height, float sigma, std::vector<float>& outImage); 


float imgGrad(const std::vector<float>& image, int width, int height, int x, int y); // 图像梯度

float maskPowCurve(float mask_in, float k); // 蒙版软硬边缘幂曲线

Vec3 heightMapToNormal(const std::vector<float>& heightMap, int width, int height, int x, int y); // 画布高度图法线计算 

void PMDiffusion(std::vector<float>& image, int width, int height, float K, float lambda, int numIter); // Perona-Malik

float hueLerp(float h1, float h2, float t); // HSL 环形色相插值

float gammaCorrection(float value, float gamma); // 伽马亮度校正
Color gammaCorrectionRGB(const Color& color, float gamma);


float normalPDF(float x, float mu, float sigma); // 正态分布随机生成


float blueNoise(float baseIntensity, float noiseVal); // 蓝噪声
// 带旋转/缩放的纹理坐标变换后采样
float blueNoiseTf(float baseIntensity, const std::vector<float>& noiseTexture,
	int texWidth, int texHeight, float x, float y, float rotation, float scale);


float lambertDiffuse(const Vec3& normal, const Vec3& lightDir, float diffuse); // Lambert 漫反射光照
Color lambertDiffuseColor(const Vec3& normal, const Vec3& lightDir, const Color& diffuseColor); // RGB 漫反射

float phongSpecular(const Vec3& normal, const Vec3& lightDir, const Vec3& viewDir, float shininess); // Phong 高光光照
// 带高光颜色的 Phong 光照
Color phongLighting(const Vec3& normal, const Vec3& lightDir, const Vec3& viewDir, float shininess, const Color& specularColor);
Color phongLightingFull(const Vec3& normal, const Vec3& lightDir, const Vec3& viewDir,
		const Color& diffuseColor, const Color& specularColor, float shininess, float ambientIntensity); 