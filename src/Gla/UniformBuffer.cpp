#include "UniformBuffer.hpp"

#include "GlaBase.hpp"

namespace Gla
{
UniformBuffer::UniformBuffer(unsigned int binding_point,
                             const void* data /*= nullptr*/)
    : mRendererID{0}, mBindingPoint{binding_point}, mSize{0},
      mBufferData{nullptr}
{
    GLCall(glGenBuffers(1, &mRendererID));
    GLCall(glBindBuffer(GL_UNIFORM_BUFFER, mRendererID));
    GLCall(glBufferData(GL_UNIFORM_BUFFER, kBlockSize, data, GL_DYNAMIC_DRAW));
}

void UniformBuffer::UpdateData(const void* data) const
{
    Bind();
    GLCall(glBufferData(GL_UNIFORM_BUFFER, kBlockSize, data, GL_DYNAMIC_DRAW));
}

void UniformBuffer::Bind() const
{
    GLCall(glBindBufferBase(GL_UNIFORM_BUFFER, mBindingPoint, mRendererID));
}
} // namespace Gla
