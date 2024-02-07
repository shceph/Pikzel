#pragma once

#include "GlaBase.hpp"

namespace Gla
{
    class VertexBuffer
    {
    private:
        unsigned int m_RendererID;
        unsigned int m_Size;  // In bytes
    public:
        VertexBuffer(const float *data, unsigned int size);
        ~VertexBuffer();

        void UpdateData(const void *data, unsigned int size, unsigned int offset = 0);
        void UpdateSize(unsigned int size);
        void UpdateSizeIfShould(unsigned int needed_size);

        void Bind() const;
        void Unbind() const;

        constexpr size_t GetSize() const { return m_Size; }
    };
}