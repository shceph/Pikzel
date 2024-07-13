#pragma once

#include "GlaBase.hpp"

namespace Gla
{
    class VertexBuffer
    {
    public:
        VertexBuffer(const void* data, std::size_t size);
        ~VertexBuffer();

        void UpdateData(const void* data, std::size_t size, std::size_t offset = 0ull);
        void UpdateSize(std::size_t size);
        void UpdateSizeIfNeeded(std::size_t needed_size);

        void Bind() const;
        void Unbind() const;

        inline std::size_t GetSize() const { return m_Size; }

    private:
        unsigned int m_RendererID;
        std::size_t m_Size;  // In bytes
    };
}