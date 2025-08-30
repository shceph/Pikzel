#pragma once

#include "pixel_buffer.hpp"

namespace Gla {
class PixelBufferRing {
  public:
    explicit PixelBufferRing(glm::ivec2 dims, int buffer_count = 2);
    void SwapBuffer();
    void ResizeBuffers(glm::ivec2 dims, Color fill_color);
    [[nodiscard]] auto GetCurrentPBO() const -> const PixelBuffer&;
    [[nodiscard]] auto GetCurrentPBO() -> PixelBuffer&;

  private:
    std::size_t mCurrentBufferIndex = 0;
    int mBufferCount;
    std::vector<PixelBuffer> mBuffers;
};
} // namespace Gla
