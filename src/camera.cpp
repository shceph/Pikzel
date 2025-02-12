#include "camera.hpp"

#include "events.hpp"

#include <algorithm>

namespace Pikzel
{
void Camera::AddToZoom(double val_to_add)
{
    mZoomValue = std::clamp(mZoomValue + val_to_add, kZoomMin, kZoomMax);
}

void Camera::SetCenter(glm::vec2 center)
{
    mCenter = center;
}

void Camera::MoveCenter(glm::vec2 offset)
{
    mCenter += offset;
}

void Camera::ResetCamera()
{
    ResetCenter();
    ResetZoom();
}

void Camera::ResetCenter()
{
    mCenter = mCanvasDims / 2;
}

void Camera::ResetZoom()
{
    mZoomValue = kZoomDefault;
}

void Camera::ScrollCallback(double /*xoffset*/, double yoffset)
{
    AddToZoom(yoffset / 30);
}

void Camera::CursorPosCallback(double x_pos, double y_pos)
{
    static double old_x = x_pos;
    static double old_y = y_pos;

    if (Events::IsMouseButtonHeld(Events::MouseButtons::kButtonRight))
    {
        glm::vec2 offset{old_x - x_pos, old_y - y_pos};
        glm::vec2 resolution_at_no_additinal_offset{500, 500};
        offset *= (glm::vec2{mCanvasDims} / resolution_at_no_additinal_offset) *
                  static_cast<float>(1.0 - mZoomValue);
        MoveCenter(offset);
    }

    old_x = x_pos;
    old_y = y_pos;
}
} // namespace Pikzel
