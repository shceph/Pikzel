#pragma once

#include "vertex_buffer.hpp"
#include "vertex_buffer_layout.hpp"

namespace Gla
{
class VertexArray
{
  public:
    VertexArray();
    VertexArray(const VertexArray&) = default;
    VertexArray(VertexArray&&) = delete;
    auto operator=(const VertexArray&) -> VertexArray& = default;
    auto operator=(VertexArray&&) -> VertexArray& = delete;
    ~VertexArray();

    void AddBuffer(const VertexBuffer& vbo,
                   const VertexBufferLayout& layout) const;
    void Bind() const;
    static void Unbind();

  private:
    unsigned int mRendererID{0};
};
} // namespace Gla
