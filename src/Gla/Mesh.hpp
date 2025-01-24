#pragma once

#include "Shader.hpp"
#include "Texture.hpp"
#include "VertexArray.hpp"

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

  private:
    const VertexArray* mVertexArray;
    const Shader* mShader;
    const Texture* mTexture;
};
} // namespace Gla
