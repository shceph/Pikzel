#include "VertexBufferLayout.hpp"

namespace Gla
{
    VertexBufferElement::VertexBufferElement(unsigned int _type, unsigned int _count, unsigned int _normalized)
        : type(_type), count(_count), normalized(_normalized) {}

    template <typename T>
    void VertexBufferLayout::Push(unsigned int count, unsigned int normalized)
    {
        throw std::logic_error("Method 'Push' in 'VertexBufferLayout' is not defined for the given type");
    }

    template <>
    void VertexBufferLayout::Push<float>(unsigned int count, unsigned int normalized)
    {
        m_Elements.push_back({ GL_FLOAT, count, normalized });
        m_Stride += VertexBufferElement::GetSizeOfType(GL_FLOAT) * count;
    }

    template <>
    void VertexBufferLayout::Push<unsigned int>(unsigned int count, unsigned int normalized)
    {
        m_Elements.push_back({ GL_UNSIGNED_INT, count, normalized });
        m_Stride += VertexBufferElement::GetSizeOfType(GL_UNSIGNED_INT) * count;
    }

    template <>
    void VertexBufferLayout::Push<unsigned char>(unsigned int count, unsigned int normalized)
    {
        m_Elements.push_back({ GL_UNSIGNED_BYTE, count, normalized });
        m_Stride += VertexBufferElement::GetSizeOfType(GL_BYTE) * count;
    }
}
