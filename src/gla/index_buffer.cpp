#include "index_buffer.hpp"

namespace Gla {
IndexBuffer::IndexBuffer(const void* data, unsigned int count,
                         GLenum type /*= GL_UNSIGNED_INT*/)
    : mRendererID{0}, mCount{count} {
    glGenBuffers(1, &mRendererID);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mRendererID);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER,
                 count * (type == GL_UNSIGNED_BYTE ? sizeof(char)
                                                   : sizeof(unsigned int)),
                 data, GL_DYNAMIC_DRAW);
}

IndexBuffer::~IndexBuffer() { glDeleteBuffers(1, &mRendererID); }

void IndexBuffer::Bind() const {
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mRendererID);
}

void IndexBuffer::Unbind() { glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0); }

void IndexBuffer::UpdateData(const void* data, unsigned int size) const {
    Bind();
    glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, size, data);
}
} // namespace Gla
