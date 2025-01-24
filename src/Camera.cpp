#include "Camera.hpp"

#include "Events.hpp"
#include "Project.hpp"

#include <algorithm>

namespace Pikzel
{
void Camera::AddToZoom(double val_to_add)
{
    sZoomValue = std::clamp(sZoomValue + val_to_add, kZoomMin, kZoomMax);
}

void Camera::SetCenter(glm::vec2 center)
{
    sCenter = center;
}

void Camera::MoveCenter(glm::vec2 offset)
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
        glm::vec2 offset{old_x - x_pos, old_y - y_pos};
        glm::vec2 resolution_at_no_additinal_offset{500, 500};
        offset *= (glm::vec2{Project::GetCanvasDims()} /
                   resolution_at_no_additinal_offset) *
                  static_cast<float>(1.0 - sZoomValue);
        Pikzel::Camera::MoveCenter(offset);
    }

    old_x = x_pos;
    old_y = y_pos;
}
} // namespace Pikzel
