#include "uniform_buffer.hpp"

#include "gla_base.hpp"

namespace Gla
{
UniformBuffer::UniformBuffer(unsigned int binding_point,
                             const void* data /*= nullptr*/)
    : mRendererID{0}, mBindingPoint{binding_point}, mSize{0},
      mBufferData{nullptr}
{
    glGenBuffers(1, &mRendererID);
    glBindBuffer(GL_UNIFORM_BUFFER, mRendererID);
    glBufferData(GL_UNIFORM_BUFFER, kBlockSize, data, GL_DYNAMIC_DRAW);
}

void UniformBuffer::UpdateData(const void* data) const
{
    Bind();
    glBufferData(GL_UNIFORM_BUFFER, kBlockSize, data, GL_DYNAMIC_DRAW);
}

void UniformBuffer::Bind() const
{
    glBindBufferBase(GL_UNIFORM_BUFFER, mBindingPoint, mRendererID);
}
} // namespace Gla
