#pragma once

#include <glm/glm.hpp>

#include <string>
#include <unordered_map>

namespace Gla
{
struct ShaderProgramSource
{
    std::string VertexSource;
    std::string FragmentSource;
};

class Shader
{
  public:
    Shader(const Shader&) = default;
    Shader(Shader&&) = delete;
    auto operator=(const Shader&) -> Shader& = default;
    auto operator=(Shader&&) -> Shader& = delete;
    explicit Shader(const std::string& filepath);
    Shader(const std::string& vertex_filepath,
           const std::string& fragment_filepath);
    ~Shader();

    void Bind() const;
    static void Unbind();
    void SetUniform1i(const std::string& name, int value);
    void SetUniform3i(const std::string& name, int val0, int val1, int val2);
    void SetUniform1f(const std::string& name, float value);
    void SetUniform3f(const std::string& name, float val0, float val1,
                      float val2);
    void SetUniform4f(const std::string& name, float val0, float val1,
                      float val2, float val3);
    void SetUniform1iv(const std::string& name, int count, int* data);
    void SetUniformMat2f(const std::string& name, const glm::mat2& matrix);
    void SetUniformMat4f(const std::string& name, const glm::mat4& matrix);
    void SetUniformMat4x2f(const std::string& name, const glm::mat4x2& matrix);
    void SetUniformMat2x4f(const std::string& name, const glm::mat2x4& matrix);
    void SetUniformMat4fv(const std::string& name, int count,
                          const float* data);

    constexpr auto GetHandle() const -> unsigned int { return mRendererID; }

  private:
    static auto ParseShader(const std::string& filepath) -> ShaderProgramSource;
    static auto
    ParseShader(const ShaderProgramSource& shader_paths) -> ShaderProgramSource;
    auto CreateShader(const std::string& vertex_shader,
                      const std::string& fragment_shader) -> unsigned int;
    auto CompileShader(unsigned int type,
                       const std::string& source) -> unsigned int;
    auto GetUniformLocation(const std::string& name) -> int;

    unsigned int mRendererID;
    std::string mFilePath;
    std::unordered_map<std::string, int> mUniformLocationCache;
};
} // namespace Gla
