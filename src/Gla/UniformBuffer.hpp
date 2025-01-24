#pragma once

namespace Gla
{
// This class is only suited for a uniform block with one 4x4 matrix
class UniformBuffer
{
  public:
    UniformBuffer(const UniformBuffer&) = default;
    UniformBuffer(UniformBuffer&&) = delete;
    auto operator=(const UniformBuffer&) -> UniformBuffer& = default;
    auto operator=(UniformBuffer&&) -> UniformBuffer& = delete;
    explicit UniformBuffer(unsigned int binding_point,
                           const void* data = nullptr);
    ~UniformBuffer() = default;

    void UpdateData(const void* data) const;
    void Bind() const;

    [[nodiscard]] auto GetHandle() const -> unsigned int { return mRendererID; }

  private:
    static constexpr unsigned int kBlockSize = 16 * sizeof(float);

    unsigned int mRendererID;
    unsigned int mBindingPoint;
    unsigned int mSize;
    char* mBufferData;
};
} // namespace Gla
