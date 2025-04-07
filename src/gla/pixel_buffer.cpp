#include "pixel_buffer.hpp"

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

auto PixelBuffer::Map() const -> PboMappedBuffSpan
{
    auto* ptr =
        static_cast<Color*>(glMapBuffer(GL_PIXEL_UNPACK_BUFFER, GL_READ_WRITE));
    assert(ptr != nullptr);
    return {ptr, mSize / sizeof(Color)};
}

auto PixelBuffer::BindAndMap() const -> PboMappedBuffSpan
{
    Bind();
    return Map();
}

void PixelBuffer::Unmap()
{
    glUnmapBuffer(GL_PIXEL_UNPACK_BUFFER);
}

void PixelBuffer::BindAndUnmap() const
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
} // namespace Gla
