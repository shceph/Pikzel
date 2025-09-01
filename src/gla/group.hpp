#pragma once

#include "shader.hpp"
#include "texture.hpp"
#include "vertex_array.hpp"
#include "vertex_buffer.hpp"

#include <cassert>

namespace Gla {
class Group {
  public:
    Group(VertexArray& vao, Shader& shader, Texture& texture)
        : mVertexArray{&vao}, mShader{&shader}, mTexture{&texture} {}

    Group(VertexArray& vao, Shader& shader)
        : mVertexArray{&vao}, mShader{&shader} {}

    Group(VertexArray& vao, VertexBuffer& vbo, Shader& shader)
        : mVertexArray{&vao}, mVertexBuffer{&vbo}, mShader{&shader} {}

    void Bind() const {
        assert(mVertexArray != nullptr && mShader != nullptr);

        mVertexArray->Bind();
        mShader->Bind();

        if (mTexture != nullptr) {
            mTexture->Bind();
        }
    }

    void Unbind() const {
        VertexArray::Unbind();
        Shader::Unbind();

        if (mTexture != nullptr) {
            mTexture->Unbind();
        }
    }

    [[nodiscard]] auto GetVao() const -> const VertexArray& {
        assert(mVertexArray != nullptr);
        return *mVertexArray;
    }

    [[nodiscard]] auto GetVao() -> VertexArray& {
        assert(mVertexArray != nullptr);
        return *mVertexArray;
    }

    [[nodiscard]] auto GetVbo() const -> const VertexBuffer& {
        assert(mVertexBuffer != nullptr);
        return *mVertexBuffer;
    }

    [[nodiscard]] auto GetVbo() -> VertexBuffer& {
        assert(mVertexBuffer != nullptr);
        return *mVertexBuffer;
    }

    [[nodiscard]] auto GetShader() const -> const Shader& {
        assert(mShader != nullptr);
        return *mShader;
    }

    [[nodiscard]] auto GetShader() -> Shader& {
        assert(mShader != nullptr);
        return *mShader;
    }

  private:
    VertexArray* mVertexArray = nullptr;
    VertexBuffer* mVertexBuffer = nullptr;
    Shader* mShader = nullptr;
    Texture* mTexture = nullptr;
};
} // namespace Gla
