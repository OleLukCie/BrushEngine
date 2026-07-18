#include "texture.h"
#include "../base/base_math.h"
#include "../base/gaussRand.h"
#include <cmath>
#include <random>

Texture::Texture() : width_(0), height_(0) {}

Texture::Texture(int width, int height)
    : width_(width), height_(height), pixels_(width * height, Color(0, 0, 0, 0)) {}

Texture::Texture(int width, int height, const Color& fillColor)
    : width_(width), height_(height), pixels_(width * height, fillColor) {}

Texture Texture::fromRGBA8(const std::vector<uint8_t>& data, int width, int height, float gamma) {
    Texture tex(width, height);
    if (data.size() < static_cast<size_t>(width * height * 4)) return tex;
    for (int i = 0; i < width * height; ++i) {
        float r = data[i * 4 + 0] / 255.0f;
        float g = data[i * 4 + 1] / 255.0f;
        float b = data[i * 4 + 2] / 255.0f;
        float a = data[i * 4 + 3] / 255.0f;
        tex.pixels_[i] = Color(
            gammaCorrection(r, 1.0f / gamma),
            gammaCorrection(g, 1.0f / gamma),
            gammaCorrection(b, 1.0f / gamma),
            a
        );
    }
    return tex;
}

Texture Texture::fromGrayscale8(const std::vector<uint8_t>& data, int width, int height) {
    Texture tex(width, height);
    if (data.size() < static_cast<size_t>(width * height)) return tex;
    for (int i = 0; i < width * height; ++i) {
        float v = data[i] / 255.0f;
        tex.pixels_[i] = Color(v, v, v, 1.0f);
    }
    return tex;
}

Texture Texture::fromFloatData(const std::vector<float>& data, int width, int height, int channels) {
    Texture tex(width, height);
    if (channels == 1) {
        for (int i = 0; i < width * height && i < static_cast<int>(data.size()); ++i) {
            float v = data[i];
            tex.pixels_[i] = Color(v, v, v, 1.0f);
        }
    } else if (channels == 4) {
        for (int i = 0; i < width * height && i * 4 + 3 < static_cast<int>(data.size()); ++i) {
            tex.pixels_[i] = Color(data[i * 4], data[i * 4 + 1], data[i * 4 + 2], data[i * 4 + 3]);
        }
    }
    return tex;
}


Color& Texture::pixel(int x, int y) {
    return pixels_[y * width_ + x];
}

const Color& Texture::pixel(int x, int y) const {
    return pixels_[y * width_ + x];
}

Color Texture::pixelSafe(int x, int y) const {
    if (x < 0 || x >= width_ || y < 0 || y >= height_) return Color(0, 0, 0, 0);
    return pixel(x, y);
}


float Texture::wrapCoordinate(float coord, int size, WrapMode mode) const {
    switch (mode) {
        case WrapMode::Clamp:
            return clamp(coord, 0.0f, static_cast<float>(size - 1));
        case WrapMode::Repeat: {
            float f = std::fmod(coord, static_cast<float>(size));
            if (f < 0) f += size;
            return f;
        }
        case WrapMode::Mirror: {
            float f = std::fmod(coord, 2.0f * size);
            if (f < 0) f += 2.0f * size;
            if (f >= size) f = 2.0f * size - 1.0f - f;
            return f;
        }
    }
    return coord;
}

int Texture::wrapIndex(int idx, int size, WrapMode mode) const {
    switch (mode) {
        case WrapMode::Clamp:
            return clamp(idx, 0, size - 1);
        case WrapMode::Repeat: {
            int r = idx % size;
            if (r < 0) r += size;
            return r;
        }
        case WrapMode::Mirror: {
            int r = idx % (2 * size);
            if (r < 0) r += 2 * size;
            if (r >= size) r = 2 * size - 1 - r;
            return clamp(r, 0, size - 1);
        }
    }
    return clamp(idx, 0, size - 1);
}


Color Texture::sampleBilinear(float u, float v, WrapMode wrap) const {
    if (empty()) return Color(0, 0, 0, 0);

    float px = u * width_ - 0.5f;
    float py = v * height_ - 0.5f;

    int x0 = static_cast<int>(std::floor(px));
    int y0 = static_cast<int>(std::floor(py));
    float fx = px - x0;
    float fy = py - y0;

    int x1 = wrapIndex(x0 + 1, width_, wrap);
    int y1 = wrapIndex(y0 + 1, height_, wrap);
    x0 = wrapIndex(x0, width_, wrap);
    y0 = wrapIndex(y0, height_, wrap);

    Color c00 = pixel(x0, y0);
    Color c10 = pixel(x1, y0);
    Color c01 = pixel(x0, y1);
    Color c11 = pixel(x1, y1);

    Color c0 = lerpColor(c00, c10, fx);
    Color c1 = lerpColor(c01, c11, fx);
    return lerpColor(c0, c1, fy);
}

Color Texture::sampleNearest(float u, float v, WrapMode wrap) const {
    if (empty()) return Color(0, 0, 0, 0);

    int x = static_cast<int>(u * width_);
    int y = static_cast<int>(v * height_);
    x = wrapIndex(x, width_, wrap);
    y = wrapIndex(y, height_, wrap);
    return pixel(x, y);
}

Color Texture::sample(float u, float v, FilterMode filter, WrapMode wrap) const {
    switch (filter) {
        case FilterMode::Nearest:
            return sampleNearest(u, v, wrap);
        case FilterMode::Bilinear:
            return sampleBilinear(u, v, wrap);
    }
    return sampleBilinear(u, v, wrap);
}

Color Texture::sampleTransformed(float u, float v, float rotation, float scale,
                                  FilterMode filter, WrapMode wrap) const {

    float cu = u - 0.5f;
    float cv = v - 0.5f;

    cu /= scale;
    cv /= scale;

    float ru, rv;
    rotate2D(cu, cv, rotation, ru, rv);

    u = ru + 0.5f;
    v = rv + 0.5f;

    return sample(u, v, filter, wrap);
}


Texture Texture::generateNoise(int width, int height, float intensity) {
    Texture tex(width, height);
    std::mt19937 rng(std::random_device{}());
    std::uniform_real_distribution<float> dist(0.0f, 1.0f);

    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            float n = dist(rng) * intensity;
            tex.pixel(x, y) = Color(n, n, n, 1.0f);
        }
    }
    return tex;
}

Texture Texture::generateBlueNoise(int width, int height, float intensity) {
    Texture tex(width, height);
    GaussRand gauss(0.5f, 0.15f);

    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            float n = clamp(gauss.generate(), 0.0f, 1.0f) * intensity;
            tex.pixel(x, y) = Color(n, n, n, 1.0f);
        }
    }
    return tex;
}

Texture Texture::generateGradient(int width, int height, const Color& c1, const Color& c2, bool horizontal) {
    Texture tex(width, height);
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            float t = horizontal ? static_cast<float>(x) / (width - 1)
                                 : static_cast<float>(y) / (height - 1);
            tex.pixel(x, y) = lerpColor(c1, c2, t);
        }
    }
    return tex;
}

Texture Texture::generateRadialGradient(int width, int height, const Color& center, const Color& edge) {
    Texture tex(width, height);
    float cx = width * 0.5f;
    float cy = height * 0.5f;
    float maxDist = std::sqrt(cx * cx + cy * cy);

    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            float dx = x - cx;
            float dy = y - cy;
            float dist = std::sqrt(dx * dx + dy * dy) / maxDist;
            tex.pixel(x, y) = lerpColor(center, edge, saturate(dist));
        }
    }
    return tex;
}


std::vector<float> Texture::toGrayscale() const {
    std::vector<float> gray(width_ * height_);
    for (int i = 0; i < width_ * height_; ++i) {
        gray[i] = pixels_[i].r * 0.299f + pixels_[i].g * 0.587f + pixels_[i].b * 0.114f;
    }
    return gray;
}

void Texture::fromGrayscale(const std::vector<float>& gray) {
    int count = std::min(static_cast<int>(gray.size()), width_ * height_);
    for (int i = 0; i < count; ++i) {
        float v = gray[i];
        pixels_[i] = Color(v, v, v, 1.0f);
    }
}

Color TextureSampler::sample(float u, float v) const {
    if (!texture) return Color(0, 0, 0, 0);
    return texture->sample(u, v, filter, wrap);
}

Color TextureSampler::sampleAtPixel(float x, float y) const {
    if (!texture) return Color(0, 0, 0, 0);
    float u = x / texture->width();
    float v = y / texture->height();
    return texture->sample(u, v, filter, wrap);
}

Color TextureSampler::sampleTransformed(float u, float v, float rotation, float scale) const {
    if (!texture) return Color(0, 0, 0, 0);
    return texture->sampleTransformed(u, v, rotation, scale, filter, wrap);
}
