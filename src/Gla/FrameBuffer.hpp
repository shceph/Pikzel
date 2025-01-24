#pragma once

#include <glm/glm.hpp>

namespace Gla
{
class FrameBuffer // For now, only stores depth, as I needed such framebuffer
                  // for shadow mapping.
{
  public:
    struct Dims
    {
        int width;
        int height;
    };

    FrameBuffer(const FrameBuffer&) = default;
    FrameBuffer(FrameBuffer&&) = delete;
    auto operator=(const FrameBuffer&) -> FrameBuffer& = default;
    auto operator=(FrameBuffer&&) -> FrameBuffer& = delete;
    explicit FrameBuffer(Dims dims);
    ~FrameBuffer();

    void Rescale(Dims dims);

    void Bind() const;
    [[nodiscard]] auto GetTextureID() const -> unsigned int
    {
        return mTextureID;
    }

    static void BindToDefaultFB();

  private:
    unsigned int mFrameBufferID;
    unsigned int mTextureID;
    Dims mDims;
};
} // namespace Gla
