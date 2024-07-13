#include "UniformBuffer.hpp"

namespace Gla
{
    UniformBuffer::UniformBuffer(unsigned int binding_point, const void* data /*= nullptr*/)
        : m_BindingPoint(binding_point)
    {
        GLCall( glGenBuffers(1, &m_RendererID) );
        GLCall( glBindBuffer(GL_UNIFORM_BUFFER, m_RendererID) );
        GLCall( glBufferData(GL_UNIFORM_BUFFER, BlockSize, data, GL_DYNAMIC_DRAW) );
    }

    void UniformBuffer::UpdateData(const void* data)
    {
        GLCall( glBufferData(GL_UNIFORM_BUFFER, BlockSize, data, GL_DYNAMIC_DRAW) );
    }

    void UniformBuffer::Bind() const
    {
        GLCall( glBindBufferBase(GL_UNIFORM_BUFFER, m_BindingPoint, m_RendererID) );
    }
}