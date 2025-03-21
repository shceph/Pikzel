#pragma once

#include "layer.hpp"
#include "layer_control.hpp"
#include "project.hpp"

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
    VertexBufferControl(Layers& layers, Vertex* ptr_to_buffer,
                        std::size_t count);
    void Map(Gla::VertexBuffer& vbo);
    static void Unmap(Gla::VertexBuffer& vbo);
    void Update(bool should_update_all,
                const std::vector<Vec2Int>& dirty_pixels);

    // Be vary; this funtion binds the vertex buffer
    void UpdateSize(Gla::VertexBuffer& vbo);
    void UpdateSizeIfNeeded(Gla::VertexBuffer& vbo);

    static void PushDirtyPixel(Vec2Int dirty_pixel);
    static void SetUpdateAllToTrue() { sUpdateAll = true; }

    [[nodiscard]] auto GetVertexCount() const -> std::size_t
    {
        return mVertexCount;
    }
    [[nodiscard]] static auto GetNeededVBOSizeForLayer(Vec2Int dims)
        -> std::size_t
    {
        return static_cast<std::size_t>(dims.x * dims.y) * kVerticesPerPixel *
               sizeof(Vertex);
    }

  private:
    std::reference_wrapper<Layers> mLayers;
    std::span<Vertex> mBufferData;
    std::size_t mVertexCount = 0;
    static inline std::vector<Vec2Int> sDirtyPixels;
    static inline bool sUpdateAll = true;
};
} // namespace Pikzel
