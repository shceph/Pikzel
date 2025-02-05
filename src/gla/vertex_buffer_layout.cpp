#include "vertex_buffer_layout.hpp"

namespace Gla
{
template <typename T>
void VertexBufferLayout::Push(unsigned int /*unused*/, unsigned int /*unused*/)
{
    throw std::logic_error("Method 'Push' in 'VertexBufferLayout' is not "
                           "defined for the given type");
}

template <>
void VertexBufferLayout::Push<float>(unsigned int count,
                                     unsigned int normalized)
{
    mElement.push_back({GL_FLOAT, count, normalized});
    mStride += VertexBufferElement::GetSizeOfType(GL_FLOAT) * count;
}

template <>
void VertexBufferLayout::Push<unsigned int>(unsigned int count,
                                            unsigned int normalized)
{
    mElement.push_back({GL_UNSIGNED_INT, count, normalized});
    mStride += VertexBufferElement::GetSizeOfType(GL_UNSIGNED_INT) * count;
}

template <>
void VertexBufferLayout::Push<unsigned char>(unsigned int count,
                                             unsigned int normalized)
{
    mElement.push_back({GL_UNSIGNED_BYTE, count, normalized});
    mStride += VertexBufferElement::GetSizeOfType(GL_BYTE) * count;
}
} // namespace Gla
