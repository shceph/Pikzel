#pragma once

#include <glm/glm.hpp>

namespace Pikzel
{
using Vec2Int = glm::vec<2, int>;

class Camera
{
  public:
    void AddToZoom(double val_to_add);
    void SetCenter(glm::vec2 center);
    void MoveCenter(glm::vec2 offset);
    void ResetCamera();
    void ResetCenter();
    void ResetZoom();
    void ScrollCallback(double xoffset, double yoffset);
    void CursorPosCallback(double x_pos, double y_pos);
    // The zoom value times width/height shows how much will be taken from
    // the width and the height of the canvas.
    [[nodiscard]] inline auto GetZoomValue() const -> double
    {
        return mZoomValue;
    }
    [[nodiscard]] inline auto GetCenter() -> glm::vec2 { return mCenter; }
    [[nodiscard]] inline auto GetCenterAsVec2Int() const -> Vec2Int
    {
        return Vec2Int{mCenter};
    }
    inline void SetCanvasDims(Vec2Int dims) { mCanvasDims = dims; }

  private:
    glm::vec2 mCenter;
    Vec2Int mCanvasDims;
    double mZoomValue = 0.0;
    constexpr static double kZoomMin = 0.0;
    constexpr static double kZoomMax = 0.93;
};
} // namespace Pikzel
