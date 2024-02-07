#pragma once

#include "GlaBase.hpp"

namespace Gla
{
    class FrameBuffer  // For now, only stores depth, as I needed such framebuffer for shadow mapping.
    {
    public:
        FrameBuffer(int width, int height);
        ~FrameBuffer();

        void Bind() const;
        unsigned int GetTextureID() const;

        static void BindToDefaultFB(int window_width, int window_height);
    private:
        unsigned int m_FrameBufferID;
        unsigned int m_TextureID;
        const int m_Width, m_Height;
    };
} // namespace Gla
