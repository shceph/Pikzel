#include "Renderer.hpp"

namespace Gla
{
    void Renderer::DrawElements(GLenum draw_mode, unsigned int indices_count, const void* indices /*= nullptr*/, GLenum type /*= GL_UNSIGNED_INT*/) const
    {
        GLCall( glDrawElements(draw_mode, indices_count, type, indices) );
    }

    void Renderer::DrawArrays(GLenum draw_mode, unsigned int vertices_count) const
    {
        GLCall( glDrawArrays(draw_mode, 0, vertices_count) );
    }

    void Renderer::Clear() const
    {
        GLCall( glClearDepth(0.0f) );
        GLCall( glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT) );
    }

    void Renderer::Flush() const
    {
        GLCall( glFlush() );
    }
}