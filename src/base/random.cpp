#include "base_math.h"
#include "gaussRand.h"

// 二维高斯模糊核
// G(x,y) = 1/(2*pi*sigma^2) * exp(-(x^2+y^2)/(2*sigma^2))
// 生成中心强、四周弱的平滑权重
float gaussian2D(
    float x,      // 水平偏移坐标
    float y,      // 垂直偏移坐标
    float sigma   // 高斯标准差（控制模糊半径，越大越模糊、扩散越广）
) {
    // 防止除零，sigma太小直接返回0
    if (sigma < 1e-6f) return 0.0f;

    float sigma2 = sigma * sigma;

    // 高斯指数部分：距离越远，值越小
    float exponent = -(x * x + y * y) / (2.0f * sigma2);

    // 返回高斯权重值
    return (1.0f / (2.0f * PI * sigma2)) * std::exp(exponent);
}

// 生成一维离散高斯核，利用可分离性
// 二维高斯 = 两个一维高斯相乘
std::vector<float> genGaussianKernel1D(
    float sigma,
    int& outRadius // 输出核半径
) {
    // 高斯有效范围： 3σ之外可忽略
    int radius = static_cast<int>(std::ceil(3.0f * sigma));
    outRadius = radius;

    // 核长度（对称分布）
    std::vector<float> kernel(2 * radius + 1);
    float sum = 0.0f;

    // 计算每个位置的高斯权重
    for (int i = -radius; i <= radius; ++i) {
        // 一维核等价于y=0的二维高斯
        float val = gaussian2D(static_cast<float>(i), 0.0f, sigma);
        kernel[i + radius] = val;
        sum += val;
    }

    // 归一化，保证模糊后亮度不变
    for (float& v : kernel) {
        v /= sum;
    }

    return kernel;
}


// 离散高斯卷积后处理
// I_blur(x,y) = sum I(x+i, y+j) * G(i,j)
// 利用高斯可分离性，先横向模糊再纵向模糊，而非直接二维卷积
void gaussianBlur(
    const std::vector<float>& image, // 输入图像（一维数组存灰度图）
    int width, // 图像宽度
    int height, // 图像高度
    float sigma, // 模糊强度
    std::vector<float>& outImage // 模糊后的图像
) {
    // 输入图像尺寸不匹配直接返回
    if (image.size() != static_cast<size_t>(width * height)) return;

    // 输出图像重置大小
    outImage.resize(image.size());

    // 生成一维高斯核，获取核半径
    int radius = 0;
    std::vector<float> kernel = genGaussianKernel1D(sigma, radius);

    // 临时图像：存储第一次水平模糊后的结果
    std::vector<float> temp(image.size());

    // 第一次：水平方向卷积
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            float sum = 0.0f;

            // 遍历高斯核左右半径范围
            for (int k = -radius; k <= radius; ++k) {
                // 采样坐标限制在图像内，防止越界
                int sx = std::max(0, std::min(width - 1, x + k));
                // 加权求和
                sum += image[y * width + sx] * kernel[k + radius];
            }

            // 存入临时图像
            temp[y * width + x] = sum;
        }
    }

    // 第二次：垂直方向卷积
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            float sum = 0.0f;

            // 遍历高斯核上下半径范围
            for (int k = -radius; k <= radius; ++k) {
                // 采样坐标限制在图像内
                int sy = std::max(0, std::min(height - 1, y + k));
                sum += temp[sy * width + x] * kernel[k + radius];
            }

            // 最终结果存入输出图像
            outImage[y * width + x] = sum;
        }
    }
}


// 正态分布随机生成
// g(x,mu,sigma) = 1/(sigma*sqrt(2*pi)) * exp(-(x-mu)^2/(2*sigma^2))
float normalPDF(
    float x,     // 输入值
    float mu,    // 均值
    float sigma  // 标准差
) {
    if (sigma < 1e-6f) return 0.0f;

    // 到中心的距离
    float diff = x - mu;

    // 方差
    float sigma2 = sigma * sigma;

    // 高斯指数部分
    float exponent = -(diff * diff) / (2.0f * sigma2);

    // 归一化系数
    float norm = 1.0f / (sigma * std::sqrt(2.0f * PI));

    return norm * std::exp(exponent);
}


// 铅笔蓝噪声强度叠加
// intensity = baseIntensity * noise(x', y')
float blueNoise(
    float baseIntensity, // 基础强度 [0,1]
    float noiseVal // 蓝噪声采样值 [0,1]
) {
    noiseVal = std::max(0.0f, std::min(1.0f, noiseVal));
    return baseIntensity * noiseVal;
}

// 带旋转/缩放的纹理坐标变换后采样
float blueNoiseTf(
    float baseIntensity,
    const std::vector<float>& noiseTexture, // 蓝噪声纹理数据
    int texWidth,            // 噪声纹理宽度
    int texHeight,           // 噪声纹理高度
    float x, float y,        // 世界坐标
    float rotation,          // 纹理旋转角度
    float scale              // 纹理缩放比例
) {
    // 坐标变换：旋转 + 缩放
    float c = std::cos(rotation);
    float s = std::sin(rotation);
    float tx = (x * c - y * s) * scale;
    float ty = (x * s + y * c) * scale;

    // 纹理坐标取模（平铺）
    int ix = static_cast<int>(std::floor(tx)) % texWidth;
    int iy = static_cast<int>(std::floor(ty)) % texHeight;

    // 处理负数坐标
    if (ix < 0) ix += texWidth;
    if (iy < 0) iy += texHeight;

    // 采样蓝噪声纹理
    float noiseVal = noiseTexture[iy * texWidth + ix];
    return baseIntensity * noiseVal;
}