#pragma once

#include "GlaBase.hpp"

namespace Gla
{
	enum VertexBufferUsage
	{
		STATIC_DRAW = GL_STATIC_DRAW,
		STREAM_DRAW = GL_STREAM_DRAW,
		DYNAMIC_DRAW = GL_DYNAMIC_DRAW
	};

    class VertexBuffer
    {
    public:
        VertexBuffer(const void* data, std::size_t size, VertexBufferUsage usage = STATIC_DRAW);
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
		VertexBufferUsage m_Usage;
    };
}
