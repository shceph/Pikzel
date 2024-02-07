#include "IndexBuffer.hpp"

namespace Gla
{
    IndexBuffer::IndexBuffer(const void *data, unsigned int count, GLenum type /*= GL_UNSIGNED_INT*/)
        : m_Count(count)
    {
        GLCall( glGenBuffers(1, &m_RendererID) );
        GLCall( glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_RendererID) );
        GLCall( glBufferData(GL_ELEMENT_ARRAY_BUFFER, count * (type == GL_UNSIGNED_BYTE? sizeof(char) : sizeof(unsigned int)), data, GL_DYNAMIC_DRAW) );
    }

    IndexBuffer::~IndexBuffer()
    {
        GLCall( glDeleteBuffers(1, &m_RendererID) );
    }

    void IndexBuffer::Bind() const
    {
        GLCall( glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_RendererID) );
    }

    void IndexBuffer::Unbind() const
    {
        GLCall( glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0) );
    }

    void IndexBuffer::UpdateData(const void *data, unsigned int size)  // Doesn't bind the array! Be caucious!!
    {
        GLCall( glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, size, data));
    }
}