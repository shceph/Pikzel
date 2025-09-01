#pragma once

#include <glad/gl.h>

#include <cstdint>

namespace Gla {
enum class DrawMode : uint8_t {
    kTriangles = GL_TRIANGLES,
    kTriangleStrip = GL_TRIANGLE_STRIP,
    kLines = GL_LINES,
    kPoints = GL_POINTS
};

class Renderer {
  public:
    // use with index buffer
    static void DrawElements(DrawMode draw_mode, GLsizei indices_count,
                             const void* indices = nullptr,
                             GLenum type = GL_UNSIGNED_INT);
    // use for drawing without index buffer
    static void DrawArrays(DrawMode draw_mode, GLsizei vertices_count);
    static void Clear();
    static void Flush();
};
} // namespace Gla
