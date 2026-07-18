#include "layer.h"

Layer::Layer(int width, int height, const std::string& name)
    // 初始化列表：按声明顺序初始化成员变量
    : name_(name), // 图层名称
      // 渲染目标
      renderTarget_(std::make_unique<CPURenderTarget>(width, height)),
      // 默认可见
      visible_(true),
      // 默认不透明
      opacity_(1.0f),
      // 默认混合模式：正常
      blendMode_(BlendMode::Normal),
      // 默认未锁定
      locked_(false) {
}
