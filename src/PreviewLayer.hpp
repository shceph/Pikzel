#pragma once

#include "Layer.hpp"
#include "Tool.hpp"
#include <glm/glm.hpp>
#include <utility>

namespace Pikzel
{
class PreviewLayer
{
  public:
    explicit PreviewLayer(std::shared_ptr<Tool> tool,
                          std::shared_ptr<Camera> camera, Vec2Int canvas_dims)
        : mTool{std::move(tool)},
          mLayer{mTool, std::move(camera), canvas_dims, false},
          mTranslationMat{0.0F}
    {
    }

    void UpdateCircleSize(int size);
    void Clear();
    void EmplaceVertices(std::vector<Vertex>& vertices);
    void Update(); // This one should run every frame
    [[nodiscard]] auto IsToolTypeChanged() const -> bool;

    [[nodiscard]] inline auto IsPreviewLayerChanged() const -> bool
    {
        return mPreviewLayerChanged;
    }
    [[nodiscard]] inline auto ShouldApplyCursorBasedTranslation() const -> bool
    {
        return mApplyCursorBasedTranslation;
    }
    inline void SetPreviewLayerChangedToTrue() { mPreviewLayerChanged = true; }

  private:
    std::shared_ptr<Tool> mTool;
    Layer mLayer;
    glm::mat4 mTranslationMat;
    Color mToolColor{0, 0, 0, 0};
    ToolType mToolType = kBrush;
    int mBrushSize = 1;
    bool mPreviewLayerChanged = true;
    bool mApplyCursorBasedTranslation = true;
};
} // namespace Pikzel
