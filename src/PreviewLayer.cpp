#include "PreviewLayer.hpp"
#include "Project.hpp"

namespace Pikzel
{
void PreviewLayer::UpdateCircleSize(int size)
{
    mLayer.Clear();
    mLayer.DrawCircle({Project::CanvasWidth() / 2, Project::CanvasHeight() / 2},
                      size, true, {100, 100, 100, 100});
    mPreviewLayerChanged = true;
}

void PreviewLayer::Clear()
{
    mLayer.Clear();
    mPreviewLayerChanged = true;
}

void PreviewLayer::EmplaceVertices(std::vector<Vertex>& vertices)
{
    mLayer.EmplaceVertices(vertices, true);
}

void PreviewLayer::Update()
{
    mPreviewLayerChanged = false;

    if (mBrushSize != Tool::GetBrushRadius())
    {
        mBrushSize = Tool::GetBrushRadius();
        UpdateCircleSize(mBrushSize);
    }

    if (mToolColor != Tool::GetColor())
    {
        mToolColor = Tool::GetColor();
        mLayer.DrawCircle(
            {Project::CanvasWidth() / 2, Project::CanvasHeight() / 2},
            mBrushSize, true, {100, 100, 100, 100});
        mPreviewLayerChanged = true;
    }
}
} // namespace Pikzel
