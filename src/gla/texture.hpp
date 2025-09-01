#pragma once

#include <glad/gl.h>

#include <glm/glm.hpp>

#include <cstddef>
#include <cstdint>
#include <array>
#include <string>

namespace Gla {
enum GLMinMagFilter : uint16_t {
    kLinear = GL_LINEAR,
    kNearest = GL_NEAREST,
};

class Texture {
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
    // NOLINTNEXTLINE(misc-non-private-member-variables-in-classes,cppcoreguidelines-non-private-member-variables-in-classes)
    unsigned int mTextureID{0};
};

class TextureCubeMap : public Texture {
  public:
    static constexpr std::size_t kCubeMapTextureCount = 6;

    TextureCubeMap(const TextureCubeMap&) = default;
    TextureCubeMap(TextureCubeMap&&) = delete;
    auto operator=(const TextureCubeMap&) -> TextureCubeMap& = default;
    auto operator=(TextureCubeMap&&) -> TextureCubeMap& = delete;
    explicit TextureCubeMap(const std::string& path);
    explicit TextureCubeMap(
        std::array<std::string, kCubeMapTextureCount> paths);
    ~TextureCubeMap() override;

    void Bind(unsigned int slot = 0) const override;
    void Unbind() const override;
};

class Texture2D : public Texture {
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
    // If param data is null, data from bound PBO is passed instead
    void UpdateWholeTexture(glm::ivec2 dims, const void* data);

    [[nodiscard]] auto GetWidth() const -> int { return mWidth; }
    [[nodiscard]] auto GetHeight() const -> int { return mHeight; }

  private:
    std::string mFilePath;
    unsigned char* mLocalBuffer;
    int mWidth, mHeight, mBPP;
};
} // namespace Gla
