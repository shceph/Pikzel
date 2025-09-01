#pragma once

#include <glad/gl.h>

#include <glm/glm.hpp>

#include <cstddef>
#include <span>

namespace Gla {
struct Color {
    GLubyte r = 0, g = 0, b = 0, a = 0;
};

using PboMappedBuffSpan = std::span<Color>;

class PixelBuffer {
  public:
    explicit PixelBuffer(glm::ivec2 dims);
    PixelBuffer(glm::ivec2 dims, Color fill_color);
    void Bind() const;
    static void Unbind();
    auto Map() -> PboMappedBuffSpan;
    auto BindAndMap() -> PboMappedBuffSpan;
    void Unmap();
    void BindAndUnmap();
    void Resize(glm::ivec2 dims, Color fill_color);
    void BindAndResize(glm::ivec2 dims, Color fill_color);
    [[nodiscard]] auto GetMappedMemory() const -> PboMappedBuffSpan;
    [[nodiscard]] auto IsMapped() const -> bool;

  private:
    GLuint mRendererID;
    std::size_t mSize;
    PboMappedBuffSpan mMappedMemory;
    bool mIsMapped{false};
};
} // namespace Gla
