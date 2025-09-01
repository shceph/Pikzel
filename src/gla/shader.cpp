#include "shader.hpp"

#include <glad/gl.h>

#include <glm/glm.hpp>

#include <cstdint>
#include <vector>
#include <fstream>
#include <sstream>
#include <array>
#include <iostream>
#include <string>

namespace Gla {
Shader::Shader(const std::string& filepath)
    : mRendererID{0}, mFilePath{filepath} {
    const ShaderProgramSource source = ParseShader(filepath);
    mRendererID = CreateShader(source.VertexSource, source.FragmentSource);
}

Shader::Shader(const std::string& vertex_filepath,
               const std::string& fragment_filepath)
    : mRendererID{0}, mFilePath{vertex_filepath} {
    const ShaderProgramSource source = ParseShader(
        {.VertexSource = vertex_filepath, .FragmentSource = fragment_filepath});
    mRendererID = CreateShader(source.VertexSource, source.FragmentSource);
}

Shader::~Shader() { glDeleteProgram(mRendererID); }

void Shader::Bind() const { glUseProgram(mRendererID); }

void Shader::Unbind() { glUseProgram(0); }

void Shader::SetUniform1i(const std::string& name, int value) {
    glUniform1i(GetUniformLocation(name), value);
}

void Shader::SetUniform3i(const std::string& name, int val0, int val1,
                          int val2) {
    glUniform3i(GetUniformLocation(name), val0, val1, val2);
}

void Shader::SetUniform1f(const std::string& name, float value) {
    glUniform1f(GetUniformLocation(name), value);
}

void Shader::SetUniform2f(const std::string& name, float val0, float val1) {
    glUniform2f(GetUniformLocation(name), val0, val1);
}

void Shader::SetUniform3f(const std::string& name, float val0, float val1,
                          float val2) {
    glUniform3f(GetUniformLocation(name), val0, val1, val2);
}

void Shader::SetUniform4f(const std::string& name, float val0, float val1,
                          float val2, float val3) {
    glUniform4f(GetUniformLocation(name), val0, val1, val2, val3);
}

void Shader::SetUniform1iv(const std::string& name, int count, int* data) {
    glUniform1iv(GetUniformLocation(name), count, data);
}

void Shader::SetUniformMat2f(const std::string& name, const glm::mat2& matrix) {
    glUniformMatrix2fv(GetUniformLocation(name), 1, GL_FALSE, &matrix[0][0]);
}

void Shader::SetUniformMat4x2f(const std::string& name,
                               const glm::mat4x2& matrix) {
    glUniformMatrix4x2fv(GetUniformLocation(name), 1, GL_FALSE, &matrix[0][0]);
}

void Shader::SetUniformMat2x4f(const std::string& name,
                               const glm::mat2x4& matrix) {
    glUniformMatrix2x4fv(GetUniformLocation(name), 1, GL_FALSE, &matrix[0][0]);
}

void Shader::SetUniformMat4f(const std::string& name, const glm::mat4& matrix) {
    glUniformMatrix4fv(GetUniformLocation(name), 1, GL_FALSE, &matrix[0][0]);
}

void Shader::SetUniformMat4fv(const std::string& name, int count,
                              const float* data) {
    glUniformMatrix4fv(GetUniformLocation(name), count, GL_FALSE, data);
}

auto Shader::GetUniformLocation(const std::string& name) -> int {
    if (mUniformLocationCache.contains(name)) {
        return mUniformLocationCache[name];
    }

    const int location = glGetUniformLocation(mRendererID, name.c_str());

    if (location == -1) {
#ifndef NDEBUG
        std::cout << "Warning: uniform '" << name << "' doesn't exist - "
                  << mFilePath << '\n';
#endif
    }

    mUniformLocationCache[name] = location;

    return location;
}

auto Shader::ParseShader(const std::string& filepath) -> ShaderProgramSource {
    std::ifstream stream(filepath);

    enum ShaderType : int8_t { kNone = -1, kVertex = 0, kFragment = 1 };

    ShaderType type = ShaderType::kNone;

    std::string line;
    std::array<std::stringstream, 2> str_stream;

    while (getline(stream, line)) {
        if (line.find("#shader") != std::string::npos) {
            if (line.find("vertex") != std::string::npos) {
                type = ShaderType::kVertex;
            } else if (line.find("fragment") != std::string::npos) {
                type = ShaderType::kFragment;
            }
        } else {
            str_stream.at(static_cast<int>(type)) << line << '\n';
        }
    }

    return {.VertexSource = str_stream[kVertex].str(),
            .FragmentSource = str_stream[kFragment].str()};
}

auto Shader::ParseShader(const ShaderProgramSource& shader_paths)
    -> ShaderProgramSource {
    std::ifstream vert_ifstream(shader_paths.VertexSource);
    std::ifstream frag_ifstream(shader_paths.FragmentSource);

    std::stringstream vert_sstream;
    std::stringstream frag_sstream;

    std::string line;

    while (getline(vert_ifstream, line)) {
        vert_sstream << line << '\n';
    }

    while (getline(frag_ifstream, line)) {
        frag_sstream << line << '\n';
    }

    return {.VertexSource = vert_sstream.str(),
            .FragmentSource = frag_sstream.str()};
}

auto Shader::CompileShader(unsigned int type, const std::string& source)
    -> unsigned int {
    const unsigned int shader_id = glCreateShader(type);
    const char* src = source.c_str();
    glShaderSource(shader_id, 1, &src, nullptr);
    glCompileShader(shader_id);

    int result = 0;
    glGetShaderiv(shader_id, GL_COMPILE_STATUS, &result);

    if (result == GL_FALSE) {
#ifndef NDEBUG
        std::cout << "Compilation error: " + mFilePath;
#endif

        int lenght = 0;
        glGetShaderiv(shader_id, GL_INFO_LOG_LENGTH, &lenght);

        std::vector<char> message(lenght);

        glGetShaderInfoLog(shader_id, lenght, &lenght, message.data());

#ifndef NDEBUG
        std::cout << message.data();
#endif

        glDeleteShader(shader_id);
        return 0;
    }

    return shader_id;
}

auto Shader::CreateShader(const std::string& vertex_shader,
                          const std::string& fragment_shader) -> unsigned int {
    const unsigned int program = glCreateProgram();

    const GLuint vert_shader = CompileShader(GL_VERTEX_SHADER, vertex_shader);
    const GLuint frag_shader =
        CompileShader(GL_FRAGMENT_SHADER, fragment_shader);

    glAttachShader(program, vert_shader);
    glAttachShader(program, frag_shader);
    glLinkProgram(program);
    glValidateProgram(program);

    glDeleteShader(vert_shader);
    glDeleteShader(frag_shader);

    return program;
}
} // namespace Gla
