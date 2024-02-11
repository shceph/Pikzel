#pragma once

#include "GlaBase.hpp"

namespace Gla
{
    class FrameBuffer  // For now, only stores depth, as I needed such framebuffer for shadow mapping.
    {
    public:
        FrameBuffer(int width, int height);
        ~FrameBuffer();

        void Rescale(int width, int height);

        void Bind() const;
        unsigned int GetTextureID() const;

        static void BindToDefaultFB();
    private:
        unsigned int m_FrameBufferID;
        unsigned int m_TextureID;
        int m_Width, m_Height;
    };
} // namespace Gla
