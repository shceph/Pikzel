#pragma once

#include "GlaBase.hpp"

namespace Gla
{
enum DrawMode
{
    kTriangles = GL_TRIANGLES,
    kLines = GL_LINES,
    kPoints = GL_POINTS
};

class Renderer
{
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
