#include "preview_layer.hpp"
#include "tool.hpp"

namespace Pikzel
{
constexpr Color kEraserToolPreviewColor{.r = 100, .g = 100, .b = 100, .a = 100};

PreviewLayer::PreviewLayer(Tool& tool, Camera& camera, Gla::VertexBuffer& vbo,
                           Vec2Int canvas_dims)
    : mTool{tool}, mVbo{vbo}, mLayer{mTool, camera, canvas_dims, false, true},
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
    mVertices.clear();
    mLayer.mOpacity = 255;
    mLayer.Clear();
    SetPreviewLayerChangedToTrue();
}

void PreviewLayer::EmplaceVertices(std::vector<Vertex>& vertices) const
{
    mLayer.EmplaceVertices(vertices, true);
}

void PreviewLayer::Update()
{
    UpdateVboIfNeeded();

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

    if (tool_type == ToolType::kRectShape ||
        tool_type == ToolType::kSelectionTool)
    {
        mApplyCursorBasedTranslation = false;
    }

    mToolType = mTool.get().GetToolType();
}

auto PreviewLayer::IsToolTypeChanged() const -> bool
{
    return mToolType != mTool.get().GetToolType();
}

void PreviewLayer::DrawRect(Vec2Int upper_left, Vec2Int bottom_right,
                            Color color)
{
    auto max_x =
        static_cast<float>(std::max(upper_left.x, bottom_right.x)) + 1.0F;
    auto min_x = static_cast<float>(std::min(upper_left.x, bottom_right.x));
    auto max_y =
        static_cast<float>(std::max(upper_left.y, bottom_right.y)) + 1.0F;
    auto min_y = static_cast<float>(std::min(upper_left.y, bottom_right.y));

    mVertices.clear();

    mVertices.emplace_back(min_x, min_y, color);
    mVertices.emplace_back(max_x, min_y, color);
    mVertices.emplace_back(min_x, max_y, color);

    mVertices.emplace_back(max_x, min_y, color);
    mVertices.emplace_back(min_x, max_y, color);
    mVertices.emplace_back(max_x, max_y, color);

    SetPreviewLayerChangedToTrue();
}

void PreviewLayer::UpdateVboIfNeeded()
{
    mVbo.get().Bind();

    if (!mPreviewLayerChanged) { return; }

    ToolType curr_tool = mTool.get().GetToolType();

    if (curr_tool != ToolType::kRectShape &&
        curr_tool != ToolType::kSelectionTool)
    {
        mVertices.clear();
        EmplaceVertices(mVertices);
        mVbo.get().UpdateSizeIfNeeded(mVertices.size() * sizeof(Vertex));
    }

    mVbo.get().UpdateData(mVertices.data(), mVertices.size() * sizeof(Vertex));
}
} // namespace Pikzel
