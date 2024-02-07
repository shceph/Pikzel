#include "FrameBuffer.hpp"

namespace Gla
{
    FrameBuffer::FrameBuffer(int width, int height)
        : m_Width(width), m_Height(height)
    {
        GLCall( glGenFramebuffers(1, &m_FrameBufferID) );
        GLCall( glBindFramebuffer(GL_FRAMEBUFFER, m_FrameBufferID) );

        GLCall( glGenTextures(1, &m_TextureID) );
        GLCall( glBindTexture(GL_TEXTURE_2D, m_TextureID) );
        GLCall( glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, m_Width, m_Height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL) );
        GLCall( glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST) );
        GLCall( glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST) );
        GLCall( glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT) );
        GLCall( glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, GL_REPEAT) );

        GLCall( glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, m_TextureID, 0) );

        GLCall( glDrawBuffer(GL_NONE) );
        GLCall( glReadBuffer(GL_NONE) );

        GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);

        if (status != GL_FRAMEBUFFER_COMPLETE)
        {
            LOG("Couldn't create FrameBuffer, error: " << GLErrorToString(status))

            #ifdef GLA_DEBUG
                throw std::logic_error("Couldn't create FrameBuffer");
            #endif // GLA_DEBUG
        }
    }
    
    FrameBuffer::~FrameBuffer()
    {
        GLCall( glDeleteFramebuffers(1, &m_FrameBufferID) );
        GLCall( glDeleteTextures(1, &m_TextureID) );
    }

    void FrameBuffer::Bind() const
    {
        GLCall( glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_FrameBufferID) );
        GLCall( glViewport(0, 0, m_Width, m_Height) );
        
        GLCall( glActiveTexture(GL_TEXTURE6) );
        GLCall( glBindTexture(GL_TEXTURE_2D, m_TextureID) );
    }

    unsigned int FrameBuffer::GetTextureID() const
    {
        return m_TextureID;
    }

    void FrameBuffer::BindToDefaultFB(int window_width, int window_height)
    {
        GLCall( glBindFramebuffer(GL_FRAMEBUFFER, 0) );
        GLCall( glViewport(0, 0, window_width, window_height) );
    }
} // namespace Gla
