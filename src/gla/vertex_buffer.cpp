#include "vertex_buffer.hpp"

#include <stdexcept>

namespace Gla
{
VertexBuffer::VertexBuffer(const void* data, std::size_t size,
                           VertexBufferUsage usage)
    : mRendererID(0), mSize(size), mUsage(usage)
{
    glGenBuffers(1, &mRendererID);
    glBindBuffer(GL_ARRAY_BUFFER, mRendererID);
    glBufferData(GL_ARRAY_BUFFER, size, data, usage);
}

VertexBuffer::~VertexBuffer()
{
    glDeleteBuffers(1, &mRendererID);
}

void VertexBuffer::UpdateData(const void* data, std::size_t size,
                              std::size_t offset /*= 0*/) const
{
    if (offset + size > mSize)
    {
        throw std::logic_error("'size + offset' is greater than allocated "
                               "size, from VertexBuffer::UpdateData");
    }

    if (offset + size < 0)
    {
        throw std::logic_error(
            "'size + offset' is lesser than 0, from VertexBuffer::UpdateData");
    }

    Bind();
    glBufferSubData(GL_ARRAY_BUFFER, offset, size, data);
}

void VertexBuffer::UpdateSize(std::size_t size) // Deletes existing data
{
    Bind();
    glBufferData(GL_ARRAY_BUFFER, size, nullptr, mUsage);
    mSize = size;
}

void VertexBuffer::UpdateSizeIfNeeded(std::size_t needed_size)
{
    if (needed_size > mSize) { UpdateSize(needed_size); }
}

void VertexBuffer::Bind() const
{
    glBindBuffer(GL_ARRAY_BUFFER, mRendererID);
}

void VertexBuffer::Unbind()
{
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}
} // namespace Gla
