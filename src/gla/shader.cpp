#include "shader.hpp"

#include "gla_base.hpp"

#include <fstream>
#include <sstream>

namespace Gla
{
Shader::Shader(const std::string& filepath)
    : mRendererID{0}, mFilePath{filepath}
{
    ShaderProgramSource source = ParseShader(filepath);
    mRendererID = CreateShader(source.VertexSource, source.FragmentSource);
}

Shader::Shader(const std::string& vertex_filepath,
               const std::string& fragment_filepath)
    : mRendererID{0}, mFilePath{vertex_filepath}
{
    ShaderProgramSource source =
        ParseShader({vertex_filepath, fragment_filepath});
    mRendererID = CreateShader(source.VertexSource, source.FragmentSource);
}

Shader::~Shader()
{
    GLCall(glDeleteProgram(mRendererID));
}

void Shader::Bind() const
{
    GLCall(glUseProgram(mRendererID));
}

void Shader::Unbind()
{
    GLCall(glUseProgram(0));
}

void Shader::SetUniform1i(const std::string& name, int value)
{
    GLCall(glUniform1i(GetUniformLocation(name), value));
}

void Shader::SetUniform3i(const std::string& name, int val0, int val1, int val2)
{
    GLCall(glUniform3i(GetUniformLocation(name), val0, val1, val2));
}

void Shader::SetUniform1f(const std::string& name, float value)
{
    GLCall(glUniform1f(GetUniformLocation(name), value));
}

void Shader::SetUniform3f(const std::string& name, float val0, float val1,
                          float val2)
{
    GLCall(glUniform3f(GetUniformLocation(name), val0, val1, val2));
}

void Shader::SetUniform4f(const std::string& name, float val0, float val1,
                          float val2, float val3)
{
    GLCall(glUniform4f(GetUniformLocation(name), val0, val1, val2, val3));
}

void Shader::SetUniform1iv(const std::string& name, int count, int* data)
{
    GLCall(glUniform1iv(GetUniformLocation(name), count, data));
}

void Shader::SetUniformMat2f(const std::string& name, const glm::mat2& matrix)
{
    GLCall(glUniformMatrix2fv(GetUniformLocation(name), 1, GL_FALSE,
                              &matrix[0][0]));
}

void Shader::SetUniformMat4x2f(const std::string& name,
                               const glm::mat4x2& matrix)
{
    GLCall(glUniformMatrix4x2fv(GetUniformLocation(name), 1, GL_FALSE,
                                &matrix[0][0]));
}

void Shader::SetUniformMat2x4f(const std::string& name,
                               const glm::mat2x4& matrix)
{
    GLCall(glUniformMatrix2x4fv(GetUniformLocation(name), 1, GL_FALSE,
                                &matrix[0][0]));
}

void Shader::SetUniformMat4f(const std::string& name, const glm::mat4& matrix)
{
    GLCall(glUniformMatrix4fv(GetUniformLocation(name), 1, GL_FALSE,
                              &matrix[0][0]));
}

void Shader::SetUniformMat4fv(const std::string& name, int count,
                              const float* data)
{
    GLCall(glUniformMatrix4fv(GetUniformLocation(name), count, GL_FALSE, data));
}

auto Shader::GetUniformLocation(const std::string& name) -> int
{
    if (mUniformLocationCache.find(name) != mUniformLocationCache.end())
    {
        return mUniformLocationCache[name];
    }

    GLCall(int location = glGetUniformLocation(mRendererID, name.c_str()));

    if (location == -1)
    {
        std::cout << "Warning: uniform '" << name << "' doesn't exist - "
                  << mFilePath << '\n';
    }

    mUniformLocationCache[name] = location;

    return location;
}

auto Shader::ParseShader(const std::string& filepath) -> ShaderProgramSource
{
    std::ifstream stream(filepath);

    enum ShaderType
    {
        kNone = -1,
        kVertex = 0,
        kFragment = 1
    };

    ShaderType type = ShaderType::kNone;

    std::string line;
    std::array<std::stringstream, 2> str_stream;

    while (getline(stream, line))
    {
        if (line.find("#shader") != std::string::npos)
        {
            if (line.find("vertex") != std::string::npos)
            {
                type = ShaderType::kVertex;
            }
            else if (line.find("fragment") != std::string::npos)
            {
                type = ShaderType::kFragment;
            }
        }
        else { str_stream.at(static_cast<int>(type)) << line << '\n'; }
    }

    return {str_stream[kVertex].str(), str_stream[kFragment].str()};
}

auto Shader::ParseShader(const ShaderProgramSource& shader_paths)
    -> ShaderProgramSource
{
    std::ifstream vert_ifstream(shader_paths.VertexSource);
    std::ifstream frag_ifstream(shader_paths.FragmentSource);

    std::stringstream vert_sstream;
    std::stringstream frag_sstream;

    std::string line;

    while (getline(vert_ifstream, line))
    {
        vert_sstream << line << '\n';
    }

    while (getline(frag_ifstream, line))
    {
        frag_sstream << line << '\n';
    }

    return {vert_sstream.str(), frag_sstream.str()};
}

auto Shader::CompileShader(unsigned int type,
                           const std::string& source) -> unsigned int
{
    unsigned int shader_id = glCreateShader(type);
    const char* src = source.c_str();
    glShaderSource(shader_id, 1, &src, nullptr);
    glCompileShader(shader_id);

    int result = 0;
    glGetShaderiv(shader_id, GL_COMPILE_STATUS, &result);

    if (result == GL_FALSE)
    {
        LOG("Compilation error: " + mFilePath);

        int lenght = 0;
        glGetShaderiv(shader_id, GL_INFO_LOG_LENGTH, &lenght);

        std::vector<char> message(lenght);

        glGetShaderInfoLog(shader_id, lenght, &lenght, message.data());
        LOG(message.data());

        GLCall(glDeleteShader(shader_id));
        return 0;
    }

    return shader_id;
}

auto Shader::CreateShader(const std::string& vertex_shader,
                          const std::string& fragment_shader) -> unsigned int
{
    GLCall(unsigned int program = glCreateProgram());

    GLuint vert_shader = CompileShader(GL_VERTEX_SHADER, vertex_shader);
    GLuint frag_shader = CompileShader(GL_FRAGMENT_SHADER, fragment_shader);

    GLCall(glAttachShader(program, vert_shader));
    GLCall(glAttachShader(program, frag_shader));
    GLCall(glLinkProgram(program));
    GLCall(glValidateProgram(program));

    GLCall(glDeleteShader(vert_shader));
    GLCall(glDeleteShader(frag_shader));

    return program;
}
} // namespace Gla
