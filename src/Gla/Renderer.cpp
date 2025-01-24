#include "Renderer.hpp"

namespace Gla
{
void Renderer::DrawElements(DrawMode draw_mode, unsigned int indices_count,
                            const void* indices /*= nullptr*/,
                            GLenum type /*= GL_UNSIGNED_INT*/)
{
    GLCall(glDrawElements(draw_mode, indices_count, type, indices));
}

void Renderer::DrawArrays(DrawMode draw_mode, std::size_t vertices_count)
{
    GLCall(glDrawArrays(draw_mode, 0, vertices_count));
}

void Renderer::Clear()
{
    /* GLCall( glClearDepth(0.0f) ); */
    GLCall(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));
}

void Renderer::Flush()
{
    GLCall(glFlush());
}
} // namespace Gla
