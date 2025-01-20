#pragma once

#include "Layer.hpp"
#include "Tool.hpp"
#include <glm/glm.hpp>

namespace Pikzel
{
class PreviewLayer
{
  public:
    PreviewLayer() : mLayer{false}, mTranslationMat{0.0F} {}

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
    Layer mLayer;
    glm::mat4 mTranslationMat;
    Color mToolColor{0, 0, 0, 0};
    ToolType mToolType = Tool::GetToolType();
    int mBrushSize = 1;
    bool mPreviewLayerChanged = true;
    bool mApplyCursorBasedTranslation = true;
};
} // namespace Pikzel
