#include "texture.hpp"

#include <stb/stb_image.h>

#include <array>
#include <cassert>
#include <iostream>

namespace Gla
{
TextureCubeMap::TextureCubeMap(const std::string& path)
{
    stbi_set_flip_vertically_on_load(0);

    int width = 0;
    int height = 0;
    int channels = 0;
    unsigned char* buffer =
        stbi_load(path.c_str(), &width, &height, &channels, 4);

    glGenTextures(1, &mTextureID);
    glBindTexture(GL_TEXTURE_CUBE_MAP, mTextureID);

    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X, 0, GL_RGBA, width, height, 0,
                 GL_RGBA, GL_UNSIGNED_BYTE, buffer);
    glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_X, 0, GL_RGBA, width, height, 0,
                 GL_RGBA, GL_UNSIGNED_BYTE, buffer);
    glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Y, 0, GL_RGBA, width, height, 0,
                 GL_RGBA, GL_UNSIGNED_BYTE, buffer);
    glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Y, 0, GL_RGBA, width, height, 0,
                 GL_RGBA, GL_UNSIGNED_BYTE, buffer);
    glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Z, 0, GL_RGBA, width, height, 0,
                 GL_RGBA, GL_UNSIGNED_BYTE, buffer);
    glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Z, 0, GL_RGBA, width, height, 0,
                 GL_RGBA, GL_UNSIGNED_BYTE, buffer);

    if (buffer != nullptr) { stbi_image_free(buffer); }
}

TextureCubeMap::TextureCubeMap(std::array<std::string, 6> paths)
{
    stbi_set_flip_vertically_on_load(0);

    std::array<int, 6> width{};
    std::array<int, 6> height{};
    std::array<int, 6> channels{};
    std::array<unsigned char*, 6> buffers{};

    for (int i = 0; i < 6; i++)
    {
        buffers.at(i) = stbi_load(paths.at(i).c_str(), &width.at(i),
                                  &height.at(i), &channels.at(i), 4);
    }

    glGenTextures(1, &mTextureID);
    glBindTexture(GL_TEXTURE_CUBE_MAP, mTextureID);

    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    for (int i = 0; i < 6; i++)
    {
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGBA,
                     width.at(i), height.at(i), 0, GL_RGBA, GL_UNSIGNED_BYTE,
                     buffers.at(i));

        if (buffers.at(i) == nullptr)
        {
            std::cout << "Warning: path no. " << i << "is null\n";
        }
        else { stbi_image_free(buffers.at(i)); }
    }
}

TextureCubeMap::~TextureCubeMap()
{
    glDeleteTextures(1, &mTextureID);
}

void TextureCubeMap::Bind(unsigned int slot /*= 0*/) const
{
    glActiveTexture(GL_TEXTURE0 + slot);
    glBindTexture(GL_TEXTURE_CUBE_MAP, mTextureID);
}

void TextureCubeMap::Unbind() const
{
    glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
}

Texture2D::Texture2D(const Texture2D& other)
    : Texture(other), mFilePath{other.mFilePath}, mLocalBuffer{nullptr},
      mWidth{other.mWidth}, mHeight{other.mHeight}, mBPP{other.mBPP}
{
    assert(other.mLocalBuffer == nullptr &&
           "Haven't made copying work with textures loaded from files, don't "
           "attempt to do so");
    mTextureID = 0;
    glGenTextures(1, &mTextureID);
    glBindTexture(GL_TEXTURE_2D, mTextureID);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
                    Gla::GLMinMagFilter::kNearest);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, mWidth, mHeight, 0, GL_RGBA,
                 GL_UNSIGNED_BYTE, nullptr);

    glCopyImageSubData(other.mTextureID, GL_TEXTURE_2D, 0, 0, 0, 0,

                       mTextureID, GL_TEXTURE_2D, 0, 0, 0, 0,

                       mWidth, mHeight, 1);

    glBindTexture(GL_TEXTURE_2D, 0);
}

auto Texture2D::operator=(const Texture2D& other) -> Texture2D&
{
    if (this == &other) { return *this; }

    assert(other.mLocalBuffer == nullptr &&
           "Haven't made copying work with textures loaded from files, don't "
           "attempt to do so");

    mTextureID = 0;
    mFilePath = other.mFilePath;
    mLocalBuffer = nullptr;
    mWidth = other.mWidth;
    mHeight = other.mHeight;
    mBPP = other.mBPP;
    glGenTextures(1, &mTextureID);
    glBindTexture(GL_TEXTURE_2D, mTextureID);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
                    Gla::GLMinMagFilter::kNearest);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glCopyImageSubData(other.mTextureID, GL_TEXTURE_2D, 0, 0, 0, 0,

                       mTextureID, GL_TEXTURE_2D, 0, 0, 0, 0,

                       mWidth, mHeight, 1);

    glBindTexture(GL_TEXTURE_2D, 0);
    return *this;
}

Texture2D::Texture2D(const std::string& path,
                     GLMinMagFilter texture_min_filter /*= LINEAR*/,
                     bool flip_vertically /*= false*/)
    : mFilePath(path), mLocalBuffer(nullptr), mWidth(0), mHeight(0), mBPP(0)
{
    stbi_set_flip_vertically_on_load(static_cast<int>(
        flip_vertically)); // Flips because opengl loads images from bottom left
    mLocalBuffer = stbi_load(path.c_str(), &mWidth, &mHeight, &mBPP, 4);
    assert(mLocalBuffer != nullptr);

#ifdef GLA_DEBUG
    if (mLocalBuffer == nullptr)
    {
        LOG("Failed to make texture, path: " << path);
    }
#endif // GLA_DEBUG

    glGenTextures(1, &mTextureID);
    glBindTexture(GL_TEXTURE_2D, mTextureID);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, texture_min_filter);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, mWidth, mHeight, 0, GL_RGBA,
                 GL_UNSIGNED_BYTE, mLocalBuffer);
    glBindTexture(GL_TEXTURE_2D, 0);

    if (mLocalBuffer != nullptr) { stbi_image_free(mLocalBuffer); }
}

Texture2D::Texture2D(glm::ivec2 dims, std::array<float, 4> fill_color,
                     GLMinMagFilter texture_min_filter)
    : mLocalBuffer{nullptr}, mWidth{dims.x}, mHeight{dims.y}, mBPP{0}
{
    glGenTextures(1, &mTextureID);
    glBindTexture(GL_TEXTURE_2D, mTextureID);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, texture_min_filter);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, mWidth, mHeight, 0, GL_RGBA,
                 GL_UNSIGNED_BYTE, nullptr);
    glClearTexImage(mTextureID, 0, GL_RGBA, GL_FLOAT, fill_color.data());

    glBindTexture(GL_TEXTURE_2D, 0);
}

Texture2D::~Texture2D()
{
    glDeleteTextures(1, &mTextureID);
}

void Texture2D::Bind(unsigned int slot /*= 0*/) const
{
    glActiveTexture(GL_TEXTURE0 + slot);
    glBindTexture(GL_TEXTURE_2D, mTextureID);
}

void Texture2D::Unbind() const
{
    glBindTexture(GL_TEXTURE_2D, 0);
}

void Texture2D::UpdatePixel(glm::ivec2 pos, std::array<GLubyte, 4> color)
{
    glTexSubImage2D(GL_TEXTURE_2D, 0, pos.x, pos.y, 1, 1, GL_RGBA,
                    GL_UNSIGNED_BYTE, color.data());
}

void Texture2D::UpdateWholeTexture(glm::ivec2 dims, const void* data)
{
    mWidth = dims.x;
    mHeight = dims.y;
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, mWidth, mHeight, 0, GL_RGBA,
                 GL_UNSIGNED_BYTE, data);
}
} // namespace Gla
