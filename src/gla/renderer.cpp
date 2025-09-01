#include "renderer.hpp"

#include <glad/gl.h>

namespace Gla {
void Renderer::DrawElements(DrawMode draw_mode, GLsizei indices_count,
                            const void* indices /*= nullptr*/,
                            GLenum type /*= GL_UNSIGNED_INT*/) {
    glDrawElements(static_cast<GLenum>(draw_mode), indices_count, type,
                   indices);
}

void Renderer::DrawArrays(DrawMode draw_mode, GLsizei vertices_count) {
    glDrawArrays(static_cast<GLenum>(draw_mode), 0, vertices_count);
}

void Renderer::Clear() {
    /*  glClearDepth(0.0f) ; */
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void Renderer::Flush() { glFlush(); }
} // namespace Gla
