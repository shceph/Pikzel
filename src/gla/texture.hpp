#pragma once

#include "gla_base.hpp"

#include <glm/glm.hpp>

#include <array>
#include <string>

namespace Gla
{
enum GLMinMagFilter
{
    kLinear = GL_LINEAR,
    kNearest = GL_NEAREST
};

class Texture
{
  public:
    Texture() = default;
    Texture(const Texture&) = default;
    Texture(Texture&&) = delete;
    auto operator=(const Texture&) -> Texture& = default;
    auto operator=(Texture&&) -> Texture& = delete;
    virtual ~Texture() = default;

    virtual void Bind(unsigned int /*slot*/ = 0) const {};
    virtual void Unbind() const {};

    [[nodiscard]] auto GetID() const -> unsigned int { return mTextureID; }

  protected:
    unsigned int mTextureID{0};
};

class TextureCubeMap : public Texture
{
  public:
    TextureCubeMap(const TextureCubeMap&) = default;
    TextureCubeMap(TextureCubeMap&&) = delete;
    auto operator=(const TextureCubeMap&) -> TextureCubeMap& = default;
    auto operator=(TextureCubeMap&&) -> TextureCubeMap& = delete;
    explicit TextureCubeMap(const std::string& path);
    explicit TextureCubeMap(std::array<std::string, 6> paths);
    ~TextureCubeMap() override;

    void Bind(unsigned int slot = 0) const override;
    void Unbind() const override;
};

class Texture2D : public Texture
{
  public:
    // Copy constructor and copy assignment operator don't handle mLocalBuffer
    // properly. For now I don't need it
    Texture2D(const Texture2D&);
    Texture2D(Texture2D&&) = delete;
    auto operator=(const Texture2D&) -> Texture2D&;
    auto operator=(Texture2D&&) -> Texture2D& = delete;
    explicit Texture2D(const std::string& path,
                       GLMinMagFilter texture_min_filter = kLinear,
                       bool flip_vertically = false);
    explicit Texture2D(glm::ivec2 dims, std::array<float, 4> fill_color,
                       GLMinMagFilter texture_min_filter = kLinear);
    ~Texture2D() override;

    void Bind(unsigned int slot = 0) const override;
    void Unbind() const override;
    static void UpdatePixel(glm::ivec2 pos, std::array<GLubyte, 4> color);
    void UpdateWholeTexture(glm::ivec2 dims, const void* data);

    [[nodiscard]] auto GetWidth() const -> int { return mWidth; }
    [[nodiscard]] auto GetHeight() const -> int { return mHeight; }

  private:
    std::string mFilePath;
    unsigned char* mLocalBuffer;
    int mWidth, mHeight, mBPP;
};
} // namespace Gla
