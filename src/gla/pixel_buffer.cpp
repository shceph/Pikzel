#include "pixel_buffer.hpp"

#ifndef NDEBUG
#include <print>
#endif

namespace Gla
{
PixelBuffer::PixelBuffer(glm::ivec2 dims)
    : mRendererID{0}, mSize{static_cast<std::size_t>(dims.x) *
                            static_cast<std::size_t>(dims.y) * 4}
{
    glGenBuffers(1, &mRendererID);
    glBindBuffer(GL_PIXEL_UNPACK_BUFFER, mRendererID);
    glBufferData(GL_PIXEL_UNPACK_BUFFER, static_cast<GLsizeiptr>(mSize),
                 nullptr, GL_STREAM_DRAW);
    glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);
}

PixelBuffer::PixelBuffer(glm::ivec2 dims, Color fill_color)
    : mRendererID{0}, mSize{static_cast<std::size_t>(dims.x) *
                            static_cast<std::size_t>(dims.y) * 4}
{
    glGenBuffers(1, &mRendererID);
    glBindBuffer(GL_PIXEL_UNPACK_BUFFER, mRendererID);
    glBufferData(GL_PIXEL_UNPACK_BUFFER, static_cast<GLsizeiptr>(mSize),
                 nullptr, GL_STREAM_DRAW);

    auto buff = Map();
    for (auto& col : buff)
    {
        col = fill_color;
    }
    Unmap();

    glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);
};

void PixelBuffer::Bind() const
{
    glBindBuffer(GL_PIXEL_UNPACK_BUFFER, mRendererID);
}

void PixelBuffer::Unbind()
{
    glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);
}

auto PixelBuffer::Map() -> PboMappedBuffSpan
{
    auto* ptr =
        static_cast<Color*>(glMapBuffer(GL_PIXEL_UNPACK_BUFFER, GL_READ_WRITE));
    assert(ptr != nullptr);
    mMappedMemory = std::span<Color>{ptr, mSize / sizeof(Color)};
    return mMappedMemory;

    // auto* ptr = static_cast<Color*>(glMapBufferRange(
    //     GL_PIXEL_UNPACK_BUFFER, 0, static_cast<GLsizeiptr>(mSize),
    //     GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT |
    //         GL_MAP_UNSYNCHRONIZED_BIT));
    // assert(ptr != nullptr);
    // mIsMapped = true;
    // mMappedMemory = std::span<Color>{ptr, mSize / sizeof(Color)};
    // return mMappedMemory;
}

auto PixelBuffer::BindAndMap() -> PboMappedBuffSpan
{
    Bind();
    return Map();
}

void PixelBuffer::Unmap()
{
    glUnmapBuffer(GL_PIXEL_UNPACK_BUFFER);
    mIsMapped = false;
}

void PixelBuffer::BindAndUnmap()
{
    Bind();
    Unmap();
}

void PixelBuffer::Resize(glm::ivec2 dims, Color fill_color)
{
    mSize = static_cast<std::size_t>(dims.x) * dims.y * 4;
    glBufferData(GL_PIXEL_UNPACK_BUFFER, static_cast<GLsizeiptr>(mSize),
                 nullptr, GL_STREAM_DRAW);

    auto buff = Map();
    for (auto& col : buff)
    {
        col = fill_color;
    }
    Unmap();
}

void PixelBuffer::BindAndResize(glm::ivec2 dims, Color fill_color)
{
    Bind();
    Resize(dims, fill_color);
}

auto PixelBuffer::GetMappedMemory() const -> PboMappedBuffSpan
{
#ifndef NDEBUG
    if (!mIsMapped)
    {
        std::println("GLA ERROR - PixelBuffer::GetMappedMemory(): The buffer "
                     "is not mapped");
    }
#endif
    return mMappedMemory;
}

auto PixelBuffer::IsMapped() const -> bool
{
    return mIsMapped;
}
} // namespace Gla
