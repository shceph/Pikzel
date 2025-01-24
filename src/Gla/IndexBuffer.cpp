#include "IndexBuffer.hpp"

namespace Gla
{
IndexBuffer::IndexBuffer(const void* data, unsigned int count,
                         GLenum type /*= GL_UNSIGNED_INT*/)
    : mRendererID{0}, mCount{count}
{
    GLCall(glGenBuffers(1, &mRendererID));
    GLCall(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mRendererID));
    GLCall(
        glBufferData(GL_ELEMENT_ARRAY_BUFFER,
                     count * (type == GL_UNSIGNED_BYTE ? sizeof(char)
                                                       : sizeof(unsigned int)),
                     data, GL_DYNAMIC_DRAW));
}

IndexBuffer::~IndexBuffer()
{
    GLCall(glDeleteBuffers(1, &mRendererID));
}

void IndexBuffer::Bind() const
{
    GLCall(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mRendererID));
}

void IndexBuffer::Unbind()
{
    GLCall(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0));
}

void IndexBuffer::UpdateData(const void* data, unsigned int size) const
{
    Bind();
    GLCall(glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, size, data));
}
} // namespace Gla
