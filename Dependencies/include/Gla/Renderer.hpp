#pragma once

#include "GlaBase.hpp"

#include "IndexBuffer.hpp"
#include "VertexBuffer.hpp"
#include "VertexBufferLayout.hpp"
#include "VertexArray.hpp"
#include "Shader.hpp"

namespace Gla
{
    class Renderer
    {
    public:
        void DrawElements(GLenum draw_mode, unsigned int indices_count, const void* indices = nullptr, GLenum type = GL_UNSIGNED_INT) const;  // use with index buffer
        void DrawArrays(GLenum draw_mode, unsigned int vertices_count) const;  // use for drawing without index buffer
        void Clear() const;
        void Flush() const;
    };
}