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
    VertexBufferControl(std::shared_ptr<Layers> layers, Vertex* ptr_to_buffer,
                        std::size_t count);
    void Update(bool should_update_all,
                const std::vector<Vec2Int>& dirty_pixels);

    // Be vary; this funtion binds the vertex buffer
    void UpdateSize(Gla::VertexBuffer& vbo);
    void UpdateSizeIfNeeded(Gla::VertexBuffer& vbo);

    static void PushDirtyPixel(Vec2Int dirty_pixel);
    static inline void SetUpdateAllToTrue() { sUpdateAll = true; }

    [[nodiscard]] inline auto GetVertexCount() const -> std::size_t
    {
        return mVertexCount;
    }
    [[nodiscard]] static inline auto
    GetNeededVBOSizeForLayer(Vec2Int dims) -> std::size_t
    {
        return static_cast<std::size_t>(dims.x * dims.y) * kVerticesPerPixel *
               sizeof(Vertex);
    }

  private:
    std::shared_ptr<Layers> mLayers;
    std::span<Vertex> mBufferData;
    std::size_t mVertexCount = 0;
    static inline std::vector<Vec2Int> sDirtyPixels;
    static inline bool sUpdateAll = true;
};
} // namespace Pikzel
