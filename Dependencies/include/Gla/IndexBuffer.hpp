#pragma once

#include "GlaBase.hpp"

namespace Gla
{
    class IndexBuffer
    {
    private:
        unsigned int m_RendererID;
        unsigned int m_Count;
    public:
        IndexBuffer(const void *data, unsigned int count, GLenum type = GL_UNSIGNED_INT);
        ~IndexBuffer();

        void Bind() const;
        void Unbind() const;

        void UpdateData(const void* data, unsigned int size);

        inline unsigned int GetCount() const { return m_Count; };
    };
}