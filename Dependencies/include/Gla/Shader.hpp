#pragma once

#include "GlaBase.hpp"
#include "UniformBuffer.hpp"

#include <fstream>
#include <sstream>
#include <string>
#include <unordered_map>

#include "../glm/gtc/matrix_transform.hpp"

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
        Shader(const std::string& filepath);
        Shader(const std::string& vertex_filepath, const std::string& fragment_filepath);
        ~Shader();

        void Bind() const;
        void Unbind() const;
        void SetUniform1i(const std::string& name, int value);
        void SetUniform3i(const std::string& name, int i0, int i1, int i2);
        void SetUniform1f(const std::string& name, float value);
        void SetUniform3f(const std::string& name, float v0, float v1, float v2);
        void SetUniform4f(const std::string& name, float v0, float v1, float v2, float v3);
        void SetUniform1iv(const std::string& name, int count, int* data);
        void SetUniformMat2f(const std::string& name, const glm::mat2& matrix);
        void SetUniformMat4f(const std::string& name, const glm::mat4& matrix);
        void SetUniformMat4x2f(const std::string& name, const glm::mat4x2& matrix);
        void SetUniformMat2x4f(const std::string& name, const glm::mat2x4& matrix);
        void SetUniformMat4fv(const std::string& name, int count, const float* data);

        constexpr const unsigned int GetHandle() const
        {
            return m_RendererID;
        }

    private:
        ShaderProgramSource ParseShader(const std::string& filepath);
        ShaderProgramSource ParseShader(const std::string& vertex_filepath, const std::string& fragment_filepath);
        unsigned int CreateShader(const std::string& vertex_shader, const std::string& fragment_shader);
        unsigned int CompileShader(unsigned int type, const std::string& source);
        int GetUniformLocation(const std::string& name);

        unsigned int m_RendererID;
        std::string m_FilePath;
        std::unordered_map<std::string, int> m_UniformLocationCache;
    };
}