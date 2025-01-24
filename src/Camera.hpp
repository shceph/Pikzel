#include "Project.hpp"
#include <glm/glm.hpp>

namespace Pikzel
{
class Camera
{
  public:
    static void AddToZoom(double val_to_add);
    static void SetCenter(glm::vec2 center);
    static void MoveCenter(glm::vec2 offset);
    static void ResetCamera();
    static void ResetCenter();
    static void ResetZoom();
    static void ScrollCallback(double xoffset, double yoffset);
    static void CursorPosCallback(double x_pos, double y_pos);
    // The zoom value times width/height shows how much will be taken from
    // the width and the height of the canvas.
    inline static auto GetZoomValue() -> double { return sZoomValue; }
    inline static auto GetCenter() -> glm::vec2 { return sCenter; }
    inline static auto GetCenterAsVec2Int() -> Vec2Int
    {
        return Vec2Int{sCenter};
    }

  private:
    inline static double sZoomValue = 0.0;
    inline static glm::vec2 sCenter;
    constexpr static double kZoomMin = 0.0;
    constexpr static double kZoomMax = 0.93;
};
} // namespace Pikzel
