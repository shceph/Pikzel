#pragma once

#include "layer.hpp"
#include "tool.hpp"
#include <glm/glm.hpp>

namespace Pikzel
{
class PreviewLayer
{
  public:
    explicit PreviewLayer(Tool& tool, Camera& camera, Vec2Int canvas_dims);

    void UpdateCircleSize(int size);
    void Clear();
    void EmplaceVertices(std::vector<Vertex>& vertices) const;
    void Update(); // This one should run every frame
    [[nodiscard]] auto IsToolTypeChanged() const -> bool;

    [[nodiscard]] auto IsPreviewLayerChanged() const -> bool
    {
        return mPreviewLayerChanged;
    }
    [[nodiscard]] auto ShouldApplyCursorBasedTranslation() const -> bool
    {
        return mApplyCursorBasedTranslation;
    }
    void SetPreviewLayerChangedToTrue() { mPreviewLayerChanged = true; }

  private:
    std::reference_wrapper<Tool> mTool;
    Layer mLayer;
    glm::mat4 mTranslationMat;
    Color mToolColor{.r = 0, .g = 0, .b = 0, .a = 0};
    ToolType mToolType = ToolType::kBrush;
    int mBrushSize = 1;
    bool mPreviewLayerChanged = true;
    bool mApplyCursorBasedTranslation = true;
};
} // namespace Pikzel
