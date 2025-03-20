#pragma once

#include "layer.hpp"
#include "tool.hpp"

#include "gla/vertex_buffer.hpp"

#include <glm/glm.hpp>

namespace Pikzel
{
class PreviewLayer
{
  public:
    explicit PreviewLayer(Tool& tool, Camera& camera, Gla::VertexBuffer& vbo,
                          Vec2Int canvas_dims);

    void UpdateCircleSize(int size);
    void Clear();
    void EmplaceVertices(std::vector<Vertex>& vertices) const;
    void Update(); // This one should run every frame
    [[nodiscard]] auto IsToolTypeChanged() const -> bool;
    void DrawRect(Vec2Int upper_left, Vec2Int bottom_right, Color color);

    [[nodiscard]] auto IsPreviewLayerChanged() const -> bool
    {
        return mPreviewLayerChanged;
    }
    [[nodiscard]] auto ShouldApplyCursorBasedTranslation() const -> bool
    {
        return mApplyCursorBasedTranslation;
    }
    [[nodiscard]] auto GetCountOfVerticesRendered() const -> std::size_t
    {
        return mVertices.size();
    }
    void SetPreviewLayerChangedToTrue() { mPreviewLayerChanged = true; }

  private:
    void UpdateVboIfNeeded();

    std::reference_wrapper<Tool> mTool;
    std::reference_wrapper<Gla::VertexBuffer> mVbo;
    std::vector<Vertex> mVertices;
    Layer mLayer;
    glm::mat4 mTranslationMat;
    Color mToolColor{.r = 0, .g = 0, .b = 0, .a = 0};
    ToolType mToolType = ToolType::kBrush;
    int mBrushSize = 1;
    bool mPreviewLayerChanged = true;
    bool mApplyCursorBasedTranslation = true;
};
} // namespace Pikzel
