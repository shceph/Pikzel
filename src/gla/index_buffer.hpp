#pragma once

#include "gla_base.hpp"

namespace Gla
{
class IndexBuffer
{
  private:
    unsigned int mRendererID;
    unsigned int mCount;

  public:
    IndexBuffer(const IndexBuffer&) = default;
    IndexBuffer(IndexBuffer&&) = delete;
    auto operator=(const IndexBuffer&) -> IndexBuffer& = default;
    auto operator=(IndexBuffer&&) -> IndexBuffer& = delete;
    IndexBuffer(const void* data, unsigned int count,
                GLenum type = GL_UNSIGNED_INT);
    ~IndexBuffer();

    void Bind() const;
    static void Unbind();

    void UpdateData(const void* data, unsigned int size) const;

    [[nodiscard]] inline auto GetCount() const -> unsigned int
    {
        return mCount;
    };
};
} // namespace Gla
