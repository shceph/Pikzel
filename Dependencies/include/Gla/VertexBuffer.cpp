#include "VertexBuffer.hpp"

namespace Gla
{
    VertexBuffer::VertexBuffer(const float* data, unsigned int size)
        : m_RendererID(0), m_Size(size)
    {
        GLCall( glGenBuffers(1, &m_RendererID) );
        GLCall( glBindBuffer(GL_ARRAY_BUFFER, m_RendererID) );
        GLCall( glBufferData(GL_ARRAY_BUFFER, size, data, GL_DYNAMIC_DRAW) );
    }

    VertexBuffer::~VertexBuffer()
    {
        GLCall( glDeleteBuffers(1, &m_RendererID) );
    }

    void VertexBuffer::UpdateData(const void* data, unsigned int size, unsigned int offset /*= 0*/)
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

    void VertexBuffer::UpdateSize(unsigned int size)  // Deletes existing data
    {
        Bind();
        GLCall( glBufferData(GL_ARRAY_BUFFER, size, nullptr, GL_DYNAMIC_DRAW) );
        m_Size = size;
    }

    void VertexBuffer::UpdateSizeIfShould(unsigned int needed_size)
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