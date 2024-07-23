#include "Camera.hpp"
#include <algorithm>

namespace Pikzel
{
void Camera::AddToZoom(double val_to_add)
{
    sZoomValue = std::clamp(sZoomValue + val_to_add, kZoomMin, kZoomMax);
}

void Camera::SetCenter(Vec2Int center)
{
    sCenter = center;
}

void Camera::MoveCenter(Vec2Int offset)
{
    sCenter += offset;
}

void Camera::ResetCamera()
{
    ResetCenter();
    ResetZoom();
}

void Camera::ResetCenter()
{
    sCenter = Project::GetCanvasDims() / 2;
}

void Camera::ResetZoom()
{
    sZoomValue = kZoomMin;
}
} // namespace Pikzel
