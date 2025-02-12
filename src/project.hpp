#pragma once

#include "camera.hpp"

#include <glm/vec2.hpp>

#include <memory>
#include <string>

namespace Pikzel
{
using Vec2Int = glm::vec<2, int>;

class Layers;
class Tool;

class Project
{
  public:
    Project(std::shared_ptr<Layers> layers, std::shared_ptr<Tool> tool,
            std::shared_ptr<Camera> camera);
    void New(Vec2Int canvas_dims);
    void Open(const std::string& project_file_dest);
    void SaveAsProject(const std::string& save_dest);
    void CloseCurrentProject();
    [[nodiscard]] auto SaveAsImage(int magnify_factor,
                                   const std::string& save_dest) const -> bool;

    [[nodiscard]] auto IsOpened() const -> bool { return mProjectOpened; }
    [[nodiscard]] auto CanvasHeight() const -> int { return mCanvasHeight; }
    [[nodiscard]] auto CanvasWidth() const -> int { return mCanvasWidth; }
    [[nodiscard]] auto GetCanvasDims() const -> Vec2Int
    {
        return {mCanvasWidth, mCanvasHeight};
    }

  private:
    std::shared_ptr<Layers> mLayers;
    std::shared_ptr<Tool> mTool;
    std::shared_ptr<Camera> mCamera;
    bool mProjectOpened = false;
    int mCanvasHeight = 0;
    int mCanvasWidth = 0;
};
} // namespace Pikzel
