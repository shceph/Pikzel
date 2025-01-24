#include "FrameBuffer.hpp"

#include "GlaBase.hpp"

namespace Gla
{
FrameBuffer::FrameBuffer(Dims dims)
    : mFrameBufferID{0}, mTextureID{0}, mDims{dims}
{
    GLCall(glGenFramebuffers(1, &mFrameBufferID));
    GLCall(glBindFramebuffer(GL_FRAMEBUFFER, mFrameBufferID));

    GLCall(glGenTextures(1, &mTextureID));
    GLCall(glBindTexture(GL_TEXTURE_2D, mTextureID));
    GLCall(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, mDims.width, mDims.height, 0,
                        GL_RGBA, GL_FLOAT, nullptr));
    GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
    GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
    GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT));
    GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, GL_REPEAT));

    GLCall(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                                  GL_TEXTURE_2D, mTextureID, 0));

    GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);

    if (status != GL_FRAMEBUFFER_COMPLETE)
    {
        LOG("Couldn't create FrameBuffer, error code: 0x" << std::hex << status)

#ifdef GLA_DEBUG
        throw std::logic_error("Couldn't create FrameBuffer");
#endif
    }
}

FrameBuffer::~FrameBuffer()
{
    GLCall(glDeleteFramebuffers(1, &mFrameBufferID));
    GLCall(glDeleteTextures(1, &mTextureID));
}

void FrameBuffer::Rescale(Dims dims)
{
    Bind();
    mDims = dims;

    GLCall(glDeleteTextures(1, &mTextureID));
    GLCall(glGenTextures(1, &mTextureID));
    GLCall(glBindTexture(GL_TEXTURE_2D, mTextureID));
    GLCall(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, mDims.width, mDims.height, 0,
                        GL_RGBA, GL_FLOAT, nullptr));
    GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
    GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
    GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT));
    GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, GL_REPEAT));

    GLCall(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                                  GL_TEXTURE_2D, mTextureID, 0));

    GLCall(glViewport(0, 0, mDims.width, mDims.height));
}

void FrameBuffer::Bind() const
{
    GLCall(glBindFramebuffer(GL_DRAW_FRAMEBUFFER, mFrameBufferID));
    // GLCall( glViewport(0, 0, m_Width, m_Height) );
    //
    // GLCall( glActiveTexture(GL_TEXTURE6) );
    // GLCall( glBindTexture(GL_TEXTURE_2D, m_TextureID) );
}

void FrameBuffer::BindToDefaultFB()
{
    GLCall(glBindFramebuffer(GL_FRAMEBUFFER, 0));
}
} // namespace Gla
