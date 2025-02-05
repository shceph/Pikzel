#pragma once

#include "gla_base.hpp"

namespace Gla
{
enum VertexBufferUsage
{
    kStaticDraw = GL_STATIC_DRAW,
    kStreamDraw = GL_STREAM_DRAW,
    kDynamicArray = GL_DYNAMIC_DRAW
};

class VertexBuffer
{
  public:
    VertexBuffer(const VertexBuffer&) = default;
    VertexBuffer(VertexBuffer&&) = delete;
    auto operator=(const VertexBuffer&) -> VertexBuffer& = default;
    auto operator=(VertexBuffer&&) -> VertexBuffer& = delete;
    VertexBuffer(const void* data, std::size_t size,
                 VertexBufferUsage usage = kStaticDraw);
    ~VertexBuffer();

    void UpdateData(const void* data, std::size_t size,
                    std::size_t offset = 0) const;
    void UpdateSize(std::size_t size);
    void UpdateSizeIfNeeded(std::size_t needed_size);

    void Bind() const;
    static void Unbind();

    [[nodiscard]] inline auto GetSize() const -> std::size_t { return mSize; }

  private:
    unsigned int mRendererID;
    std::size_t mSize; // In bytes
    VertexBufferUsage mUsage;
};
} // namespace Gla
