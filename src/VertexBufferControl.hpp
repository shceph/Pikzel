#pragma once

#include "Layer.hpp"
#include "Project.hpp"

#include <span>

namespace Gla
{
class VertexBuffer;
}

namespace Pikzel
{
constexpr int kVerticesPerPixel = 6;

class VertexBufferControl
{
  public:
    // Should run this after creating/opening a project
    VertexBufferControl(Vertex* ptr_to_buffer, std::size_t count);
    void Update(bool should_update_all,
                const std::vector<Vec2Int>& dirty_pixels);
    void UpdatePixel(Vec2Int coords);
    // Be vary; this funtion binds the vertex buffer
    void UpdateSize(Gla::VertexBuffer& vbo);
    void UpdateSizeIfNeeded(Gla::VertexBuffer& vbo);

    static void PushDirtyPixel(Vec2Int dirty_pixel);
    static inline void SetUpdateAllToTrue() { sUpdateAll = true; }

    [[nodiscard]] inline auto GetVertexCount() const -> std::size_t
    {
        return mVertexCount;
    }
    [[nodiscard]] static inline auto GetNeededVBOSizeForLayer() -> std::size_t
    {
        assert(Project::IsOpened());
        return static_cast<std::size_t>(Project::CanvasWidth() *
                                        Project::CanvasHeight()) *
               kVerticesPerPixel * sizeof(Vertex);
    }

  private:
    static inline std::vector<Vec2Int> sDirtyPixels;
    std::span<Vertex> mBufferData;
    std::size_t mVertexCount = 0;
    static inline bool sUpdateAll = true;
};
} // namespace Pikzel
