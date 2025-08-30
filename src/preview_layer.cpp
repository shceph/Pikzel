#include "preview_layer.hpp"

#include <cstddef>
#include <span>

#include "gla/pixel_buffer.hpp"
#include "gla/texture.hpp"

#include "layer.hpp"
#include "camera.hpp"
#include "tool.hpp"

namespace Pikzel {
constexpr Color kEraserToolPreviewColor{.r = 100, .g = 100, .b = 100, .a = 100};

PreviewLayer::PreviewLayer(Tool& tool, Camera& camera, Vec2 canvas_dims)
    : mTool{tool},
      mLayer{mTool,       camera, mSelection, std::span<Gla::Color>{},
             canvas_dims, false},
      mTranslationMat{0.0F} {}

void PreviewLayer::UpdateCircleSize(int size) {
    mLayer.Clear();
    mLayer.DrawCircle(mLayer.GetCanvasDims() / 2, size, Layer::DrawType::kFill,
                      kEraserToolPreviewColor);
    SetPreviewLayerChangedToTrue();
}

void PreviewLayer::Clear() {
    mLayer.mOpacity = 255;
    mLayer.Clear();
    SetPreviewLayerChangedToTrue();
}

void PreviewLayer::Update() {
    mPreviewLayerChanged = false;
    mApplyCursorBasedTranslation = true;

    auto tool_type = mTool.get().GetToolType();
    auto tool_curr_color = Color::FromImVec4(mTool.get().GetColor());

    if (tool_type == ToolType::kEraser) {
        mToolColor = kEraserToolPreviewColor;
    } else if (tool_type == ToolType::kBrush && mToolColor != tool_curr_color) {
        mToolColor = tool_curr_color;
        mLayer.DrawCircle(mLayer.GetCanvasDims() / 2, mBrushSize,
                          Layer::DrawType::kFill,
                          {.r = 100, .g = 100, .b = 100, .a = 100});
        SetPreviewLayerChangedToTrue();
    }

    if ((tool_type == ToolType::kBrush || tool_type == ToolType::kEraser) &&
        (IsToolTypeChanged() || mBrushSize != mTool.get().GetBrushRadius())) {
        mBrushSize = mTool.get().GetBrushRadius();
        UpdateCircleSize(mBrushSize);
    } else if (IsToolTypeChanged()) {
        Clear();
    }

    if (tool_type == ToolType::kRectShape ||
        tool_type == ToolType::kSelectionTool) {
        mApplyCursorBasedTranslation = false;
    }

    mToolType = mTool.get().GetToolType();
}

auto PreviewLayer::IsToolTypeChanged() const -> bool {
    return mToolType != mTool.get().GetToolType();
}

void PreviewLayer::DrawRect(Vec2 upper_left, Vec2 bottom_right, Color color) {
    mLayer.DrawRect(upper_left, bottom_right, Layer::DrawType::kFill, color);
    SetPreviewLayerChangedToTrue();
}

void PreviewLayer::DrawPixel(std::size_t index, Color color) {
    mLayer.DrawPixel(index, color);
    SetPreviewLayerChangedToTrue();
}

void PreviewLayer::DrawPixel(Vec2 coords, Color color) {
    mLayer.DrawPixel(coords, color);
    SetPreviewLayerChangedToTrue();
}

void PreviewLayer::UpdatePBOMappedBufferSpan(Gla::PboMappedBuffSpan buff) {
    mLayer.UpdateMappedPBOBufferSpan(buff);
}

auto PreviewLayer::GetLayerTexture() -> Gla::Texture2D& {
    return mLayer.GetTexture();
}
} // namespace Pikzel
