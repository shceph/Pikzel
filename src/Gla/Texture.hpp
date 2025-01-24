#pragma once

#include "GlaBase.hpp"

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

    [[nodiscard]] inline auto GetID() const -> unsigned int
    {
        return mRendererID;
    }

  protected:
    unsigned int mRendererID{0};
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
    Texture2D(const Texture2D&) = default;
    Texture2D(Texture2D&&) = delete;
    auto operator=(const Texture2D&) -> Texture2D& = default;
    auto operator=(Texture2D&&) -> Texture2D& = delete;
    explicit Texture2D(const std::string& path,
                       GLMinMagFilter texture_min_filter = kLinear,
                       bool flip_vertically = false);
    ~Texture2D() override;

    void Bind(unsigned int slot = 0) const override;
    void Unbind() const override;

    [[nodiscard]] inline auto GetWidth() const -> int { return mWidth; }
    [[nodiscard]] inline auto GetHeight() const -> int { return mHeight; }

  private:
    std::string mFilePath;
    unsigned char* mLocalBuffer;
    int mWidth, mHeight, mBPP;
};
} // namespace Gla
