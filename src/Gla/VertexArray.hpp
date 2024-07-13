#pragma once

#include "GlaBase.hpp"

#include "VertexBuffer.hpp"
#include "VertexBufferLayout.hpp"

namespace Gla
{
    class VertexArray
    {
    public:
        VertexArray();
        ~VertexArray();

        void AddBuffer(const VertexBuffer& vb, const VertexBufferLayout& layout);
        void Bind() const;
        void Unbind() const;

    private:
        unsigned int m_RendererID;
    };
}