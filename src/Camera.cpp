#include "Camera.hpp"

#include "Events.hpp"

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

void Camera::ScrollCallback(double /*xoffset*/, double yoffset)
{
    AddToZoom(yoffset / 30);
}

void Camera::CursorPosCallback(double x_pos, double y_pos)
{
    static double old_x = x_pos;
    static double old_y = y_pos;

    if (Events::IsMouseButtonHeld(Events::kButtonRight))
    {
        double offset_x = old_x - x_pos;
        double offset_y = old_y - y_pos;
        Pikzel::Camera::MoveCenter(
            {static_cast<int>(offset_x), static_cast<int>(offset_y)});
    }

    old_x = x_pos;
    old_y = y_pos;
}
} // namespace Pikzel
