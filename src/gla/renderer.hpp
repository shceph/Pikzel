#pragma once

#include "gla_base.hpp"

#include <cstdint>

namespace Gla {
enum class DrawMode : std::uint8_t {
    kTriangles = GL_TRIANGLES,
    kTriangleStrip = GL_TRIANGLE_STRIP,
    kLines = GL_LINES,
    kPoints = GL_POINTS
};

class Renderer {
  public:
    // use with index buffer
    static void DrawElements(DrawMode draw_mode, unsigned int indices_count,
                             const void* indices = nullptr,
                             GLenum type = GL_UNSIGNED_INT);
    // use for drawing without index buffer
    static void DrawArrays(DrawMode draw_mode, std::size_t vertices_count);
    static void Clear();
    static void Flush();
};
} // namespace Gla
