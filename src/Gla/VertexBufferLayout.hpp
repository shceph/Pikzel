#pragma once

#include "GlaBase.hpp"

#include <vector>

namespace Gla
{
    struct VertexBufferElement
    {
        VertexBufferElement(unsigned int _type, unsigned int _count, unsigned int _normalized);

        unsigned int type;
        unsigned int count;
        unsigned int normalized;

        static constexpr unsigned int GetSizeOfType(unsigned int _type)
        {
            switch (_type) {
                case GL_FLOAT:          return sizeof(GLfloat);
                case GL_UNSIGNED_INT:   return sizeof(GLuint);
                case GL_BYTE:           return sizeof(GLbyte);
			 case GL_UNSIGNED_BYTE:  return sizeof(GLubyte);
			 default:				GLAssert(false);
            }
            
            return 0;
        }
    };

    class VertexBufferLayout
    {
    public:
        VertexBufferLayout()
            : m_Stride(0) {}

        template <typename T>
        void Push(unsigned int count, unsigned int normalized = GL_FALSE);

        inline const std::vector<VertexBufferElement> GetElements() const { return m_Elements; };
        inline unsigned int GetStride() const { return m_Stride; };

    private:
        std::vector<VertexBufferElement> m_Elements;
        unsigned int m_Stride;
    };
}
