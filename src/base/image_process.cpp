#include "base_math.h"

// 各向异性水墨扩散（Perona-Malik）
// I(t+1) = I(t) + div(D(|grad_I|) * grad_I)
// 普通模糊会糊掉轮廓
void PMDiffusion(
    std::vector<float>& image, // 灰度图像
    int width, // 图像宽度
    int height, // 图像高度
    float K, // 边缘检测阈值
    float lambda, // 时间步长（稳定性要求 lambda <= 0.25）
    int numIter // 迭代次数
) {
    // 安全检查
    if (image.size() != static_cast<size_t>(width * height)) return;
    
    // 稳定性限制
    lambda = std::min(0.25f, lambda);

    // 临时图像缓冲区，存储下一帧结果
    std::vector<float> temp(image.size());

    // 自动钳位在图像范围内
    auto getPixel = [&](const std::vector<float>& img, int x, int y) -> float {
        x = std::max(0, std::min(width - 1, x));
        y = std::max(0, std::min(height - 1, y));
        return img[y * width + x];
    };

    // 扩散系数函数：D(s) = 1 / (1 + (s/K)^2)
    // 梯度小，系数大，扩散强烈，平滑
    // 梯度大，系数小，扩散停止，边缘
    auto diffusionCoeff = [&](float grad) -> float {
        float s = grad / K;
        return 1.0f / (1.0f + s * s);
    };

    // 迭代扩散过程
    for (int iter = 0; iter < numIter; ++iter) {
        // 遍历每个像素
        for (int y = 0; y < height; ++y) {
            for (int x = 0; x < width; ++x) {
                float I = image[y * width + x]; // 当前像素值

                // 获取上下左右四个相邻像素
                float I_n = getPixel(image, x, y - 1);
                float I_s = getPixel(image, x, y + 1);
                float I_e = getPixel(image, x + 1, y);
                float I_w = getPixel(image, x - 1, y);

                // 计算四个方向的梯度
                float gradN = I_n - I;
                float gradS = I_s - I;
                float gradE = I_e - I;
                float gradW = I_w - I;

                // 计算各方向扩散系数
                float cN = diffusionCoeff(std::abs(gradN));
                float cS = diffusionCoeff(std::abs(gradS));
                float cE = diffusionCoeff(std::abs(gradE));
                float cW = diffusionCoeff(std::abs(gradW));

                // 离散散度：扩散流量总和
                float divergence = cN * gradN + cS * gradS + cE * gradE + cW * gradW;

                // 更新像素
                temp[y * width + x] = I + lambda * divergence;
            }
        }

        // 交换缓冲区
        image.swap(temp);
    }
}


// 图像梯度
// grad_I = sqrt((dI/dx)^2 + (dI/dy)^2)
// 用于检测画面色彩边缘
float imgGrad(
    const std::vector<float>& image, // 输入灰度图像（行优先）
    int width, // 图像宽度
    int height, // 图像高度
    int x, // 待计算梯度的像素坐标
    int y
) {
    // Sobel 算子
    // Gx: [-1 0 1; -2 0 2; -1 0 1] 水平梯度，检测左右明暗变化
    // Gy: [-1 -2 -1; 0 0 0; 1 2 1] 垂直梯度，检测上下明暗变化

    // 自动把坐标限制在图像范围内，防止越界崩溃
    auto getPixel = [&](int px, int py) -> float {
        px = std::max(0, std::min(width - 1, px));
        py = std::max(0, std::min(height - 1, py));
        return image[py * width + px];
    };

    // 计算水平方向梯度
    float gx =
        -1.0f * getPixel(x - 1, y - 1) + 1.0f * getPixel(x + 1, y - 1)
        -2.0f * getPixel(x - 1, y    ) + 2.0f * getPixel(x + 1, y    )
        -1.0f * getPixel(x - 1, y + 1) + 1.0f * getPixel(x + 1, y + 1);

    // 计算垂直方向梯度
    float gy =
        -1.0f * getPixel(x - 1, y - 1) - 2.0f * getPixel(x, y - 1) - 1.0f * getPixel(x + 1, y - 1)
        + 1.0f * getPixel(x - 1, y + 1) + 2.0f * getPixel(x, y + 1) + 1.0f * getPixel(x + 1, y + 1);

    // 合并x、y梯度，返回最终边缘强度
    return std::sqrt(gx * gx + gy * gy);
}


// 蒙版软硬边缘幂曲线
// mask_out = mask_in^k
float maskPowCurve(
    float mask_in, // 蒙版值
    float k        // 幂指数
) {
    mask_in = std::max(0.0f, std::min(1.0f, mask_in));
    if (k < 1e-6f) return mask_in;
    return std::pow(mask_in, k);
}


// 画布高度图法线计算
Vec3 heightMapToNormal(
    const std::vector<float>& heightMap, // 高度图数据
    int width, // 高度图的宽
    int height, // 高度图的高
    int x, // 当前要计算法线的像素坐标
    int y
) {
    // 坐标钳位
    auto getH = [&](int px, int py) -> float {
        px = std::max(0, std::min(width - 1, px));
        py = std::max(0, std::min(height - 1, py));
        // 二维坐标转一维数组索引
        return heightMap[py * width + px];
    };

    // 中心差分计算偏导数
    float dh_dx = (getH(x + 1, y) - getH(x - 1, y)) * 0.5f;
    float dh_dy = (getH(x, y + 1) - getH(x, y - 1)) * 0.5f;

    // 法线向量: [-dh/dx, -dh/dy, 1]
    Vec3 normal(-dh_dx, -dh_dy, 1.0f);

    // 法线向量转单位向量
    return normal.normalized();
}