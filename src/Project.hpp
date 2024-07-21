#pragma once

#include <glm/vec2.hpp>
#include <string>

namespace App
{
using Vec2Int = glm::vec<2, int>;

class Project
{
  public:
    static void New(Vec2Int canvas_dims);
    static void Open(const std::string& project_file_dest);
    static void SaveAsImage(int magnify_factor, const std::string& save_dest);
    static void SaveAsProject(const std::string& save_dest);
    static void CloseCurrentProject();

    inline static auto IsOpened() -> bool { return sProjectOpened; }
    inline static auto CanvasHeight() -> int { return sCanvasHeight; }
    inline static auto CanvasWidth() -> int { return sCanvasWidth; }
    inline static auto GetCanvasDims() -> Vec2Int
    {
        return {sCanvasWidth, sCanvasHeight};
    }

  private:
    inline static bool sProjectOpened = false;
    inline static int sCanvasHeight = 0;
    inline static int sCanvasWidth = 0;
};
} // namespace App
