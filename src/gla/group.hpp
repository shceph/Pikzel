#pragma once

#include "shader.hpp"
#include "texture.hpp"
#include "vertex_array.hpp"

namespace Gla
{
class Group
{
  public:
    Group(const VertexArray& vao, const Shader& shader, const Texture& texture)
        : mVertexArray{&vao}, mShader{&shader}, mTexture{&texture}
    {
    }

    Group(const VertexArray& vao, const Shader& shader)
        : mVertexArray{&vao}, mShader{&shader}, mTexture{nullptr}
    {
    }

    Group(const VertexArray& vao, const VertexBuffer& vbo, const Shader& shader)
        : mVertexArray{&vao}, mVertexBuffer{&vbo}, mShader{&shader},
          mTexture{nullptr}
    {
    }

    void Bind() const
    {
        assert(mVertexArray != nullptr && mShader != nullptr);

        mVertexArray->Bind();
        mShader->Bind();

        if (mTexture != nullptr) { mTexture->Bind(); }
    }

    void Unbind() const
    {
        VertexArray::Unbind();
        Shader::Unbind();

        if (mTexture != nullptr) { mTexture->Unbind(); }
    }

    auto GetVao() -> const VertexArray&
    {
        assert(mVertexArray != nullptr);
        return *mVertexArray;
    }

    auto GetVbo() -> const VertexBuffer&
    {
        assert(mVertexBuffer != nullptr);
        return *mVertexBuffer;
    }

    auto GetShader() -> const Shader&
    {
        assert(mShader != nullptr);
        return *mShader;
    }

  private:
    const VertexArray* mVertexArray;
    const VertexBuffer* mVertexBuffer = nullptr;
    const Shader* mShader;
    const Texture* mTexture;
};
} // namespace Gla
