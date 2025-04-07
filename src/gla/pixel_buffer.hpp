#pragma once

#include "gla_base.hpp"

#include <glm/glm.hpp>

#include <span>

namespace Gla
{
struct Color
{
    GLubyte r = 0, g = 0, b = 0, a = 0;
};

using PboMappedBuffSpan = std::span<Color>;

class PixelBuffer
{
  public:
    explicit PixelBuffer(glm::ivec2 dims);
    PixelBuffer(glm::ivec2 dims, Color fill_color);
    void Bind() const;
    static void Unbind();
    [[nodiscard]] auto Map() const -> PboMappedBuffSpan;
    [[nodiscard]] auto BindAndMap() const -> PboMappedBuffSpan;
    static void Unmap();
    void BindAndUnmap() const;
    void Resize(glm::ivec2 dims, Color fill_color);
    void BindAndResize(glm::ivec2 dims, Color fill_color);

  private:
    GLuint mRendererID;
    std::size_t mSize;
};
} // namespace Gla
