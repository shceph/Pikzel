#include "preview_layer.hpp"
#include "tool.hpp"

namespace Pikzel
{
constexpr Color kEraserToolPreviewColor{.r = 100, .g = 100, .b = 100, .a = 100};

PreviewLayer::PreviewLayer(Tool& tool, Camera& camera, Vec2Int canvas_dims)
    : mTool{tool}, mLayer{mTool, camera, canvas_dims, false, true},
      mTranslationMat{0.0F}
{
}

void PreviewLayer::UpdateCircleSize(int size)
{
    mLayer.Clear();
    mLayer.DrawCircle(mLayer.GetCanvasDims() / 2, size, true,
                      kEraserToolPreviewColor);
    SetPreviewLayerChangedToTrue();
}

void PreviewLayer::Clear()
{
    mLayer.Clear();
    SetPreviewLayerChangedToTrue();
}

void PreviewLayer::EmplaceVertices(std::vector<Vertex>& vertices) const
{
    mLayer.EmplaceVertices(vertices, true);
}

void PreviewLayer::Update()
{
    mPreviewLayerChanged = false;
    mApplyCursorBasedTranslation = true;

    auto tool_type = mTool.get().GetToolType();
    auto tool_curr_color = Color::FromImVec4(mTool.get().GetColor());

    if (tool_type == ToolType::kEraser)
    {
        mToolColor = kEraserToolPreviewColor;
    }
    else if (tool_type == ToolType::kBrush && mToolColor != tool_curr_color)
    {
        mToolColor = tool_curr_color;
        mLayer.DrawCircle(mLayer.GetCanvasDims() / 2, mBrushSize, true,
                          {.r = 100, .g = 100, .b = 100, .a = 100});
        SetPreviewLayerChangedToTrue();
    }

    if ((tool_type == ToolType::kBrush || tool_type == ToolType::kEraser) &&
        (IsToolTypeChanged() || mBrushSize != mTool.get().GetBrushRadius()))
    {
        mBrushSize = mTool.get().GetBrushRadius();
        UpdateCircleSize(mBrushSize);
    }
    else if (IsToolTypeChanged()) { Clear(); }

    if (tool_type == ToolType::kRectShape)
    {
        Clear();
        mLayer.HandleRectShape();
        SetPreviewLayerChangedToTrue();
        mApplyCursorBasedTranslation = false;
    }

    mToolType = mTool.get().GetToolType();
}

auto PreviewLayer::IsToolTypeChanged() const -> bool
{
    return mToolType != mTool.get().GetToolType();
}
} // namespace Pikzel
