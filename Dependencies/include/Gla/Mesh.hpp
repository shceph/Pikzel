#pragma once

#include "GlaBase.hpp"
#include "VertexArray.hpp"
#include "Shader.hpp"
#include "Texture.hpp"

namespace Gla
{
    class Mesh
    {
    public:
        Mesh(const VertexArray& va, const Shader& shader, const Texture& texture)
            : m_VertexArray(va), m_Shader(shader), m_Texture(&texture) {}

        Mesh(const VertexArray& va, const Shader& shader)
            : m_VertexArray(va), m_Shader(shader), m_Texture(nullptr) {}

        void Bind() const
        {
            m_VertexArray.Bind();
            m_Shader.Bind();

            if (m_Texture != nullptr)
                m_Texture->Bind();
        }

        void Unbind() const  // Just binds to 0
        {
            m_VertexArray.Unbind();
            m_Shader.Unbind();
            
            if (m_Texture != nullptr) {
                m_Texture->Unbind();
            }
        }

    private:
        const VertexArray& m_VertexArray;
        const Shader& m_Shader;
        const Texture* m_Texture;
    };
}