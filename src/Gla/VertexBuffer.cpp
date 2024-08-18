#include "VertexBuffer.hpp"

namespace Gla
{
    VertexBuffer::VertexBuffer(const void* data, std::size_t size, VertexBufferUsage usage)
        : m_RendererID(0), m_Size(size), m_Usage(usage)
    {
        GLCall( glGenBuffers(1, &m_RendererID) );
        GLCall( glBindBuffer(GL_ARRAY_BUFFER, m_RendererID) );
        GLCall( glBufferData(GL_ARRAY_BUFFER, size, data, usage) );
    }

    VertexBuffer::~VertexBuffer()
    {
        GLCall( glDeleteBuffers(1, &m_RendererID) );
    }

    void VertexBuffer::UpdateData(const void* data, std::size_t size, std::size_t offset /*= 0*/)
    {
        if (offset + size > m_Size) {
            throw std::logic_error("'size + offset' is greater than allocated size, from VertexBuffer::UpdateData");
        }

        if (offset + size < 0) {
            throw std::logic_error("'size + offset' is lesser than 0, from VertexBuffer::UpdateData");
        }

        Bind();
        GLCall( glBufferSubData(GL_ARRAY_BUFFER, offset, size, data));
    }

    void VertexBuffer::UpdateSize(std::size_t size)  // Deletes existing data
    {
        Bind();
        GLCall( glBufferData(GL_ARRAY_BUFFER, size, nullptr, m_Usage) );
        m_Size = size;
    }

    void VertexBuffer::UpdateSizeIfNeeded(std::size_t needed_size)
    {
        if (needed_size > m_Size) {
            UpdateSize(needed_size);
        }
    }

    void VertexBuffer::Bind() const
    {
        GLCall( glBindBuffer(GL_ARRAY_BUFFER, m_RendererID) );
    }

    void VertexBuffer::Unbind() const
    {
        GLCall( glBindBuffer(GL_ARRAY_BUFFER, 0) );
    }
}
