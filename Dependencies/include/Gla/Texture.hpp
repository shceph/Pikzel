#pragma once

#include "GlaBase.hpp"

namespace Gla
{
    enum GLMinMagFilter { LINEAR = GL_LINEAR, NEAREST = GL_NEAREST };

    class Texture
    {
    public:
        virtual ~Texture() = default;

        virtual void Bind(unsigned int slot = 0) const {}
        virtual void Unbind() const {}

        inline unsigned int GetID() const { return m_RendererID; }

    protected:
        unsigned int m_RendererID;
    };

    class TextureCubeMap : public Texture
    {
    public:
        TextureCubeMap(const std::string& path);
        TextureCubeMap(const std::string paths[6]);
        ~TextureCubeMap();

        void Bind(unsigned int slot = 0) const;
        void Unbind() const;
    };

    class Texture2D : public Texture
    {
    public:
        Texture2D(const std::string& path, GLMinMagFilter texture_min_filter = LINEAR);
        ~Texture2D();

        void Bind(unsigned int slot = 0) const;
        void Unbind() const;

        inline int GetWidth()  const { return m_Width;  }
        inline int GetHeight() const { return m_Height; }

    private:
        std::string m_FilePath;
        unsigned char *m_LocalBuffer;
        int m_Width, m_Height, m_BPP;
    };
}