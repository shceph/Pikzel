#pragma once

#include "gla_base.hpp"

#include <cassert>
#include <vector>

namespace Gla
{
struct VertexBufferElement
{
    unsigned int type;
    unsigned int count;
    unsigned int normalized;

    static constexpr auto GetSizeOfType(unsigned int _type) -> unsigned int
    {
        switch (_type)
        {
        case GL_FLOAT:
            return sizeof(GLfloat);
        case GL_UNSIGNED_INT:
            return sizeof(GLuint);
        case GL_BYTE:
            return sizeof(GLbyte);
        case GL_UNSIGNED_BYTE:
            return sizeof(GLubyte);
        default:
            assert(false);
        }

        return 0;
    }
};

class VertexBufferLayout
{
  public:
    VertexBufferLayout() = default;

    template <typename T>
    void Push(unsigned int count, unsigned int normalized = GL_FALSE);

    [[nodiscard]] inline auto
    GetElements() const -> std::vector<VertexBufferElement>
    {
        return mElement;
    };
    [[nodiscard]] inline auto GetStride() const -> unsigned int
    {
        return mStride;
    };

  private:
    std::vector<VertexBufferElement> mElement;
    unsigned int mStride{};
};
} // namespace Gla
