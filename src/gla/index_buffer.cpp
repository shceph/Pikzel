#include "index_buffer.hpp"

#include <glad/gl.h>

namespace Gla {
IndexBuffer::IndexBuffer(const void* data, GLsizeiptr count,
                         GLenum type /*= GL_UNSIGNED_INT*/)
    : mRendererID{0}, mCount{count} {
    glGenBuffers(1, &mRendererID);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mRendererID);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER,
                 count * static_cast<GLsizeiptr>(type == GL_UNSIGNED_BYTE
                                                     ? sizeof(char)
                                                     : sizeof(unsigned int)),
                 data, GL_DYNAMIC_DRAW);
}

IndexBuffer::~IndexBuffer() { glDeleteBuffers(1, &mRendererID); }

void IndexBuffer::Bind() const {
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mRendererID);
}

void IndexBuffer::Unbind() { glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0); }

void IndexBuffer::UpdateData(const void* data, GLuint size) const {
    Bind();
    glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, size, data);
}
} // namespace Gla
