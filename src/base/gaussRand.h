#pragma once

#include <random> // 随机数

// Box-Muller 变换：从均匀随机数生成正态分布随机数
class GaussRand {
public:
    // 指定均值和标准差
    GaussRand(float mu = 0.0f, float sigma = 1.0f)
        : mu_(mu), sigma_(sigma), hasSpare_(false), spare_(0.0f) {}

    // 生成一个高斯随机数
    float generate() {
        // 如果有缓存的备用值，直接返回（Box-Muller一次生成2个）
        if (hasSpare_) {
            hasSpare_ = false;
            return spare_ * sigma_ + mu_;
        }

        float u1, u2, s;
        // 生成单位圆内的均匀随机点
        do {
            u1 = dist_(rng_) * 2.0f - 1.0f; // [-1,1]
            u2 = dist_(rng_) * 2.0f - 1.0f; // [-1,1]
            s = u1 * u1 + u2 * u2; // 半径平方
        } while (s >= 1.0f || s < 1e-6f); // 只保留单位圆内且不为0

        // Box-Muller
        float mul = std::sqrt(-2.0f * std::log(s) / s);

        // 生成第一个（立即返回），第二个存起来备用
        spare_ = u2 * mul;
        hasSpare_ = true;
        return u1 * mul * sigma_ + mu_;
    }

private:
    float mu_, sigma_;
    bool hasSpare_; // 是否有备用随机数
    float spare_; // 备用随机数
    // mt19937梅森旋转算法伪随机数生成器
    std::mt19937 rng_{std::random_device{}()};
    // 把mt19937生成的乱数，转成0~1之间均匀的小数
    std::uniform_real_distribution<float> dist_{0.0f, 1.0f};
};