#include "renderer.hpp"

namespace Gla
{
void Renderer::DrawElements(DrawMode draw_mode, unsigned int indices_count,
                            const void* indices /*= nullptr*/,
                            GLenum type /*= GL_UNSIGNED_INT*/)
{
    glDrawElements((GLenum)draw_mode, indices_count, type, indices);
}

void Renderer::DrawArrays(DrawMode draw_mode, std::size_t vertices_count)
{
    glDrawArrays((GLenum)draw_mode, 0, vertices_count);
}

void Renderer::Clear()
{
    /*  glClearDepth(0.0f) ; */
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void Renderer::Flush()
{
    glFlush();
}
} // namespace Gla
