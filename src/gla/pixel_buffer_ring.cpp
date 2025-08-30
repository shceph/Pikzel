#include "pixel_buffer_ring.hpp"

namespace Gla {
PixelBufferRing::PixelBufferRing(glm::ivec2 dims, int buffer_count)
    : mBufferCount{buffer_count} {
    mBuffers.reserve(buffer_count);

    for (int i = 0; i < buffer_count; i++) {
        mBuffers.emplace_back(dims);
    }
}

void PixelBufferRing::SwapBuffer() {
    mCurrentBufferIndex = (mCurrentBufferIndex + 1) % mBufferCount;
}

void PixelBufferRing::ResizeBuffers(glm::ivec2 dims, Color fill_color) {
    for (auto& buff : mBuffers) {
        buff.BindAndResize(dims, fill_color);
    }
}

auto PixelBufferRing::GetCurrentPBO() const -> const PixelBuffer& {
    return mBuffers[mCurrentBufferIndex];
}

auto PixelBufferRing::GetCurrentPBO() -> PixelBuffer& {
    return mBuffers[mCurrentBufferIndex];
}
} // namespace Gla
