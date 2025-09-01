#include "frame_buffer.hpp"

#include <glad/gl.h>

#include <stdexcept>
#include <iostream>

namespace Gla {
FrameBuffer::FrameBuffer(Dims dims)
    : mFrameBufferID{0}, mTextureID{0}, mDims{dims} {
    glGenFramebuffers(1, &mFrameBufferID);
    glBindFramebuffer(GL_FRAMEBUFFER, mFrameBufferID);

    glGenTextures(1, &mTextureID);
    glBindTexture(GL_TEXTURE_2D, mTextureID);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, mDims.width, mDims.height, 0,
                 GL_RGBA, GL_FLOAT, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, GL_REPEAT);

    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
                           mTextureID, 0);

    const GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);

    if (status != GL_FRAMEBUFFER_COMPLETE) {
        std::cout << "Couldn't create FrameBuffer, error code: 0x" << std::hex
                  << status;

        throw std::logic_error("Couldn't create FrameBuffer");
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

FrameBuffer::~FrameBuffer() {
    glDeleteFramebuffers(1, &mFrameBufferID);
    glDeleteTextures(1, &mTextureID);
}

void FrameBuffer::Rescale(Dims dims) {
    Bind();
    mDims = dims;

    glDeleteTextures(1, &mTextureID);
    glGenTextures(1, &mTextureID);
    glBindTexture(GL_TEXTURE_2D, mTextureID);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, mDims.width, mDims.height, 0,
                 GL_RGBA, GL_FLOAT, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, GL_REPEAT);

    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
                           mTextureID, 0);

    glViewport(0, 0, mDims.width, mDims.height);
}

void FrameBuffer::Bind() const {
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, mFrameBufferID);
    // glViewport(0, 0, m_Width, m_Height);
    //
    // glActiveTexture(GL_TEXTURE6);
    // glBindTexture(GL_TEXTURE_2D, m_TextureID);
}

void FrameBuffer::BindToDefaultFB() { glBindFramebuffer(GL_FRAMEBUFFER, 0); }
} // namespace Gla
