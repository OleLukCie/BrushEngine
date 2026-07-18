#include "tool_base.h"

Vec2 Tool::toCanvasSpace(const InputSample& sample, const RenderContext& ctx) const {
    return Vec2(
        (sample.x - ctx.canvasOffset.x) / ctx.canvasScale,
        (sample.y - ctx.canvasOffset.y) / ctx.canvasScale
    );
}

Vec2 Tool::toScreenSpace(const Vec2& canvasPos, const RenderContext& ctx) const {
    return Vec2(
        canvasPos.x * ctx.canvasScale + ctx.canvasOffset.x,
        canvasPos.y * ctx.canvasScale + ctx.canvasOffset.y
    );
}