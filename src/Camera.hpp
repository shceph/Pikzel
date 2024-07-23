#include "Project.hpp"

namespace Pikzel
{
class Camera
{
  public:
    static void AddToZoom(double val_to_add);
    static void SetCenter(Vec2Int center);
    static void MoveCenter(Vec2Int offset);
    static void ResetCamera();
    static void ResetCenter();
    static void ResetZoom();
    inline static auto GetZoomValue() -> double { return sZoomValue; }
    inline static auto GetCenter() -> Vec2Int { return sCenter; }

  private:
    inline static double sZoomValue = 0.0;
    inline static Vec2Int sCenter;
    constexpr static double kZoomMin = 0.0;
    constexpr static double kZoomMax = 0.93;
};
} // namespace Pikzel
