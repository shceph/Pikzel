#pragma once

#include <glad/gl.h>

namespace Gla {
class IndexBuffer {
  public:
    IndexBuffer(const IndexBuffer&) = default;
    IndexBuffer(IndexBuffer&&) = delete;
    auto operator=(const IndexBuffer&) -> IndexBuffer& = default;
    auto operator=(IndexBuffer&&) -> IndexBuffer& = delete;
    IndexBuffer(const void* data, GLsizeiptr count,
                GLenum type = GL_UNSIGNED_INT);
    ~IndexBuffer();

    void Bind() const;
    static void Unbind();

    void UpdateData(const void* data, unsigned int size) const;

    [[nodiscard]] auto GetCount() const -> GLsizeiptr { return mCount; };

  private:
    GLuint mRendererID;
    GLsizeiptr mCount;
};
} // namespace Gla
