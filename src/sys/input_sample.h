#pragma once

#include "../base/base_types.h"
#include <cstdint>

// 输入采样数据：统一封装来自数位板/鼠标/触控笔的输入
struct InputSample {
    float x = 0.0f;           // 画布坐标 X
    float y = 0.0f;           // 画布坐标 Y
    float pressure = 1.0f;    // 压感值 [0,1]
    float tiltX = 0.0f;       // X轴倾斜角度 [-90,90]
    float tiltY = 0.0f;       // Y轴倾斜角度 [-90,90]
    float timestamp = 0.0f;   // 时间戳（毫秒）
    uint64_t sequence = 0;    // 序列号，用于排序

    InputSample() = default;
    InputSample(float x_, float y_, float pressure_ = 1.0f,
                float tiltX_ = 0.0f, float tiltY_ = 0.0f,
                float timestamp_ = 0.0f, uint64_t seq = 0)
        : x(x_), y(y_), pressure(pressure_),
          tiltX(tiltX_), tiltY(tiltY_),
          timestamp(timestamp_), sequence(seq) {}

    // 转换为 Vec2
    Vec2 pos() const { return Vec2(x, y); }
};


InputSample makeInputSample(int x, int y, float pressure = 1.0f);