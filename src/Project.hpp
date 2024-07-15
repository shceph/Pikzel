#pragma once

#include <string>

namespace App
{
class Project
{
  public:
    static void New(int canvas_heigth, int canvas_width);
    static void Open(const std::string& project_file_dest);
    static void SaveAsImage(int magnify_factor, const std::string& save_dest);
    static void SaveAsProject(const std::string& save_dest);
    static void CloseCurrentProject();

    inline static auto IsOpened() -> bool { return sProjectOpened; }
    inline static auto CanvasHeight() -> int { return sCanvasHeight; }
    inline static auto CanvasWidth() -> int { return sCanvasWidth; }

  private:
    inline static bool sProjectOpened = false;
    inline static int sCanvasHeight = 0;
    inline static int sCanvasWidth = 0;
};
} // namespace App
