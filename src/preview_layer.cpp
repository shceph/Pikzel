#include "preview_layer.hpp"
#include "tool.hpp"

namespace Pikzel
{
constexpr Color kEraserToolPreviewColor{.r = 100, .g = 100, .b = 100, .a = 100};

PreviewLayer::PreviewLayer(std::shared_ptr<Tool> tool,
                           std::shared_ptr<Camera> camera, Vec2Int canvas_dims)
    : mTool{std::move(tool)},
      mLayer{mTool, std::move(camera), canvas_dims, false},
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

void PreviewLayer::EmplaceVertices(std::vector<Vertex>& vertices)
{
    mLayer.EmplaceVertices(vertices, true);
}

void PreviewLayer::Update()
{
    mPreviewLayerChanged = false;
    mApplyCursorBasedTranslation = true;

    auto tool_type = mTool->GetToolType();
    auto tool_curr_color = Color::FromImVec4(mTool->GetColor());

    if (tool_type == kEraser) { mToolColor = kEraserToolPreviewColor; }
    else if (tool_type == kBrush && mToolColor != tool_curr_color)
    {
        mToolColor = tool_curr_color;
        mLayer.DrawCircle(mLayer.GetCanvasDims() / 2, mBrushSize, true,
                          {.r = 100, .g = 100, .b = 100, .a = 100});
        SetPreviewLayerChangedToTrue();
    }

    if ((tool_type == kBrush || tool_type == kEraser) &&
        (IsToolTypeChanged() || mBrushSize != mTool->GetBrushRadius()))
    {
        mBrushSize = mTool->GetBrushRadius();
        UpdateCircleSize(mBrushSize);
    }
    else if (IsToolTypeChanged()) { Clear(); }

    if (tool_type == kRectShape)
    {
        Clear();
        mLayer.HandleRectShape();
        SetPreviewLayerChangedToTrue();
        mApplyCursorBasedTranslation = false;
    }

    mToolType = mTool->GetToolType();
}

auto PreviewLayer::IsToolTypeChanged() const -> bool
{
    return mToolType != mTool->GetToolType();
}
} // namespace Pikzel
