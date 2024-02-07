#include "Texture.hpp"

#include "../stb/stb_image.h"

namespace Gla
{
    TextureCubeMap::TextureCubeMap(const std::string& path)
    {
        stbi_set_flip_vertically_on_load(false);

        int width, height, channels;
        unsigned char* buffer = stbi_load(path.c_str(), &width, &height, &channels, 4);

        GLCall( glGenTextures(1, &m_RendererID) );
        GLCall( glBindTexture(GL_TEXTURE_CUBE_MAP, m_RendererID) );

        GLCall( glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR) );
        GLCall( glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR) );
        GLCall( glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE) );
        GLCall( glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE) );
        GLCall( glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE) );

        GLCall( glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, buffer) );
        GLCall( glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_X, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, buffer) );
        GLCall( glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Y, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, buffer) );
        GLCall( glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Y, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, buffer) );
        GLCall( glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Z, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, buffer) );
        GLCall( glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Z, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, buffer) );

        if (buffer)
            stbi_image_free(buffer);
    }

    TextureCubeMap::TextureCubeMap(const std::string paths[6])
    {   
        stbi_set_flip_vertically_on_load(false);
        
        int width[6], height[6], channels[6];
        unsigned char* buffers[6];

        for (int i = 0; i < 6; i++) {
            buffers[i] = stbi_load(paths[i].c_str(), &width[i], &height[i], &channels[i], 4);
        }

        GLCall( glGenTextures(1, &m_RendererID) );
        GLCall( glBindTexture(GL_TEXTURE_CUBE_MAP, m_RendererID) );

        GLCall( glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR) );
        GLCall( glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR) );
        GLCall( glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE) );
        GLCall( glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE) );
        GLCall( glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE) );

        for (int i = 0; i < 6; i++) {
            GLCall( glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGBA, width[i], height[i], 0, GL_RGBA, GL_UNSIGNED_BYTE, buffers[i]) );

            if (!buffers[i]) 
                std::cout << "Warning: path no. " << i << "is null\n";
            else 
                stbi_image_free(buffers[i]);
        }
    }

    TextureCubeMap::~TextureCubeMap()
    {
        GLCall( glDeleteTextures(1, &m_RendererID) );
    }

    void TextureCubeMap::Bind(unsigned int slot /*= 0*/) const
    {
        GLCall( glActiveTexture(GL_TEXTURE0 + slot) );
        GLCall( glBindTexture(GL_TEXTURE_CUBE_MAP, m_RendererID) );
    }

    void TextureCubeMap::Unbind() const
    {
        GLCall( glBindTexture(GL_TEXTURE_CUBE_MAP, 0) );
    }

    /* Texture2D */

    Texture2D::Texture2D(const std::string &path, GLMinMagFilter texture_min_filter /*= LINEAR*/)
        : m_FilePath(path), m_LocalBuffer(nullptr), m_Width(0), m_Height(0), m_BPP(0)
    {
        stbi_set_flip_vertically_on_load(1);  // Flips because opengl loads images from bottom left
        m_LocalBuffer = stbi_load(path.c_str(), &m_Width, &m_Height, &m_BPP, 4);

        #ifdef GLA_DEBUG
            if (!m_LocalBuffer) {
                LOG("Failed to make texture, path: " << path);
            }
        #endif // GLA_DEBUG

        GLCall( glGenTextures(1, &m_RendererID) );
        GLCall( glBindTexture(GL_TEXTURE_2D, m_RendererID) );

        GLCall( glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, texture_min_filter) );
        GLCall( glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST) );
        GLCall( glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE) );
        GLCall( glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE) );

        GLCall( glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, m_Width, m_Height, 0, GL_RGBA, GL_UNSIGNED_BYTE, m_LocalBuffer)  );
        GLCall( glBindTexture(GL_TEXTURE_2D, 0) );

        if (m_LocalBuffer) {
            stbi_image_free(m_LocalBuffer);
        }
    }

    Texture2D::~Texture2D()
    {
        GLCall( glDeleteTextures(1, &m_RendererID) );
    }

    void Texture2D::Bind(unsigned int slot /*= 0*/) const
    {
        GLCall( glActiveTexture(GL_TEXTURE0 + slot) );
        GLCall( glBindTexture(GL_TEXTURE_2D, m_RendererID) );
    }

    void Texture2D::Unbind() const
    {
        GLCall( glBindTexture(GL_TEXTURE_2D, 0) );
    }
}