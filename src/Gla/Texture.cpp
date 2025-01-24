#include "Texture.hpp"

#include "../stb/stb_image.h"

#include <array>

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

    GLCall(glGenTextures(1, &mRendererID));
    GLCall(glBindTexture(GL_TEXTURE_CUBE_MAP, mRendererID));

    GLCall(
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
    GLCall(
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
    GLCall(glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R,
                           GL_CLAMP_TO_EDGE));
    GLCall(glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S,
                           GL_CLAMP_TO_EDGE));
    GLCall(glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T,
                           GL_CLAMP_TO_EDGE));

    GLCall(glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X, 0, GL_RGBA, width,
                        height, 0, GL_RGBA, GL_UNSIGNED_BYTE, buffer));
    GLCall(glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_X, 0, GL_RGBA, width,
                        height, 0, GL_RGBA, GL_UNSIGNED_BYTE, buffer));
    GLCall(glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Y, 0, GL_RGBA, width,
                        height, 0, GL_RGBA, GL_UNSIGNED_BYTE, buffer));
    GLCall(glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Y, 0, GL_RGBA, width,
                        height, 0, GL_RGBA, GL_UNSIGNED_BYTE, buffer));
    GLCall(glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Z, 0, GL_RGBA, width,
                        height, 0, GL_RGBA, GL_UNSIGNED_BYTE, buffer));
    GLCall(glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Z, 0, GL_RGBA, width,
                        height, 0, GL_RGBA, GL_UNSIGNED_BYTE, buffer));

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

    GLCall(glGenTextures(1, &mRendererID));
    GLCall(glBindTexture(GL_TEXTURE_CUBE_MAP, mRendererID));

    GLCall(
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
    GLCall(
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
    GLCall(glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R,
                           GL_CLAMP_TO_EDGE));
    GLCall(glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S,
                           GL_CLAMP_TO_EDGE));
    GLCall(glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T,
                           GL_CLAMP_TO_EDGE));

    for (int i = 0; i < 6; i++)
    {
        GLCall(glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGBA,
                            width.at(i), height.at(i), 0, GL_RGBA,
                            GL_UNSIGNED_BYTE, buffers.at(i)));

        if (buffers.at(i) == nullptr)
        {
            std::cout << "Warning: path no. " << i << "is null\n";
        }
        else { stbi_image_free(buffers.at(i)); }
    }
}

TextureCubeMap::~TextureCubeMap()
{
    GLCall(glDeleteTextures(1, &mRendererID));
}

void TextureCubeMap::Bind(unsigned int slot /*= 0*/) const
{
    GLCall(glActiveTexture(GL_TEXTURE0 + slot));
    GLCall(glBindTexture(GL_TEXTURE_CUBE_MAP, mRendererID));
}

void TextureCubeMap::Unbind() const
{
    GLCall(glBindTexture(GL_TEXTURE_CUBE_MAP, 0));
}

/* Texture2D */

Texture2D::Texture2D(const std::string& path,
                     GLMinMagFilter texture_min_filter /*= LINEAR*/,
                     bool flip_vertically /*= false*/)
    : mFilePath(path), mLocalBuffer(nullptr), mWidth(0), mHeight(0), mBPP(0)
{
    stbi_set_flip_vertically_on_load(static_cast<int>(
        flip_vertically)); // Flips because opengl loads images from bottom left
    mLocalBuffer = stbi_load(path.c_str(), &mWidth, &mHeight, &mBPP, 4);

#ifdef GLA_DEBUG
    if (mLocalBuffer == nullptr)
    {
        LOG("Failed to make texture, path: " << path);
    }
#endif // GLA_DEBUG

    GLCall(glGenTextures(1, &mRendererID));
    GLCall(glBindTexture(GL_TEXTURE_2D, mRendererID));

    GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
                           texture_min_filter));
    GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST));
    GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE));
    GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE));

    GLCall(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, mWidth, mHeight, 0, GL_RGBA,
                        GL_UNSIGNED_BYTE, mLocalBuffer));
    GLCall(glBindTexture(GL_TEXTURE_2D, 0));

    if (mLocalBuffer != nullptr) { stbi_image_free(mLocalBuffer); }
}

Texture2D::~Texture2D()
{
    GLCall(glDeleteTextures(1, &mRendererID));
}

void Texture2D::Bind(unsigned int slot /*= 0*/) const
{
    GLCall(glActiveTexture(GL_TEXTURE0 + slot));
    GLCall(glBindTexture(GL_TEXTURE_2D, mRendererID));
}

void Texture2D::Unbind() const
{
    GLCall(glBindTexture(GL_TEXTURE_2D, 0));
}
} // namespace Gla
