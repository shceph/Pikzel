#pragma once

#include "GlaBase.hpp"

#include "IndexBuffer.hpp"
#include "VertexBuffer.hpp"
#include "VertexBufferLayout.hpp"
#include "VertexArray.hpp"
#include "Shader.hpp"

namespace Gla
{
    enum DrawMode { TRIANGLES = GL_TRIANGLES, LINES = GL_LINES, POINTS = GL_POINTS };

    class Renderer
    {
    public:
        void DrawElements(DrawMode draw_mode, unsigned int indices_count, const void* indices = nullptr, GLenum type = GL_UNSIGNED_INT) const;  // use with index buffer
        void DrawArrays(DrawMode draw_mode, std::size_t vertices_count) const;  // use for drawing without index buffer
        void Clear() const;
        void Flush() const;
    };
}