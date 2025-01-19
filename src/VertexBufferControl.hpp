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
    static void Init(Vertex* ptr_to_buffer, std::size_t count);
    static void Update();
    static void PushDirtyPixel(Vec2Int dirty_pixel);
    static void UpdatePixel(Vec2Int coords);
    // Be vary; this funtion binds the vertex buffer
    static void UpdateSize(Gla::VertexBuffer& vbo);
	static void UpdateSizeIfNeeded(Gla::VertexBuffer& vbo);

    static inline void SetUpdateAllToTrue() { sUpdateAll = true; }

    [[nodiscard]] static inline auto GetVertexCount() -> std::size_t
    {
        return sVertexCount;
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
    static inline std::span<Vertex> sBufferData;
    static inline std::size_t sVertexCount = 0;
    static inline bool sUpdateAll = true;
};
} // namespace Pikzel
