#include "PreviewLayer.hpp"
#include "Project.hpp"
#include "Tool.hpp"

namespace Pikzel
{
constexpr Color kEraserToolPreviewColor{100, 100, 100, 100};

void PreviewLayer::UpdateCircleSize(int size)
{
    mLayer.Clear();
    mLayer.DrawCircle({Project::CanvasWidth() / 2, Project::CanvasHeight() / 2},
                      size, true, kEraserToolPreviewColor);
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

    auto tool_type = Tool::GetToolType();

    if (tool_type == kEraser) { mToolColor = kEraserToolPreviewColor; }
    else if (tool_type == kBrush && mToolColor != Tool::GetColor())
    {
        mToolColor = Tool::GetColor();
        mLayer.DrawCircle(
            {Project::CanvasWidth() / 2, Project::CanvasHeight() / 2},
            mBrushSize, true, {100, 100, 100, 100});
        SetPreviewLayerChangedToTrue();
    }

    if ((tool_type == kBrush || tool_type == kEraser) &&
        (IsToolTypeChanged() || mBrushSize != Tool::GetBrushRadius()))
    {
        mBrushSize = Tool::GetBrushRadius();
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

    mToolType = Tool::GetToolType();
}

auto PreviewLayer::IsToolTypeChanged() const -> bool
{
    return mToolType != Tool::GetToolType();
}
} // namespace Pikzel
