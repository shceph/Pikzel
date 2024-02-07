#include "Shader.hpp"

namespace Gla
{
    Shader::Shader(const std::string& filepath)
        : m_FilePath(filepath), m_RendererID(0)
    {
        ShaderProgramSource source = ParseShader(filepath);
        m_RendererID = CreateShader(source.VertexSource, source.FragmentSource);
    }

    Shader::Shader(const std::string& vertex_filepath, const std::string& fragment_filepath)
        : m_FilePath(vertex_filepath), m_RendererID(0)
    {
        ShaderProgramSource source = ParseShader(vertex_filepath, fragment_filepath);
        m_RendererID = CreateShader(source.VertexSource, source.FragmentSource);
    }

    Shader::~Shader()
    {
        GLCall( glDeleteProgram(m_RendererID) );
    }

    void Shader::Bind() const
    {
        GLCall( glUseProgram(m_RendererID) );
    }

    void Shader::Unbind() const
    {
        GLCall( glUseProgram(0) );
    }

    void Shader::SetUniform1i(const std::string& name, int value)
    {
        GLCall( glUniform1i(GetUniformLocation(name), value) );
    }

    void Shader::SetUniform3i(const std::string& name, int i0, int i1, int i2)
    {
        GLCall( glUniform3i(GetUniformLocation(name), i0, i1, i2) );
    }

    void Shader::SetUniform1f(const std::string& name, float value)
    {
        GLCall( glUniform1f(GetUniformLocation(name), value) );
    }

    void Shader::SetUniform3f(const std::string& name, float v0, float v1, float v2)
    {
        GLCall( glUniform3f(GetUniformLocation(name), v0, v1, v2) );
    }

    void Shader::SetUniform4f(const std::string& name, float v0, float v1, float v2, float v3)
    {
        GLCall( glUniform4f(GetUniformLocation(name), v0, v1, v2, v3) );
    }

    void Shader::SetUniform1iv(const std::string &name, int count, int *data)
    {
        GLCall( glUniform1iv(GetUniformLocation(name), count, data) );
    }

    void Shader::SetUniformMat2f(const std::string& name, const glm::mat2& matrix)
    {
        GLCall( glUniformMatrix2fv(GetUniformLocation(name), 1, GL_FALSE, &matrix[0][0]) );
    }

    void Shader::SetUniformMat4x2f(const std::string& name, const glm::mat4x2& matrix)
    {
        GLCall( glUniformMatrix4x2fv(GetUniformLocation(name), 1, GL_FALSE, &matrix[0][0]) );
    }

    void Shader::SetUniformMat2x4f(const std::string& name, const glm::mat2x4& matrix)
    {
        GLCall( glUniformMatrix2x4fv(GetUniformLocation(name), 1, GL_FALSE, &matrix[0][0]) );    
    }
    
    void Shader::SetUniformMat4f(const std::string& name, const glm::mat4& matrix)
    {
        GLCall( glUniformMatrix4fv(GetUniformLocation(name), 1, GL_FALSE, &matrix[0][0]) );
    }
    
    void Shader::SetUniformMat4fv(const std::string& name, int count, const float* data)
    {
        GLCall( glUniformMatrix4fv(GetUniformLocation(name), count, GL_FALSE, data) );
    }

    int Shader::GetUniformLocation(const std::string& name)
    {
        if (m_UniformLocationCache.find(name) != m_UniformLocationCache.end())  // if found
            return m_UniformLocationCache[name];

        GLCall( int location = glGetUniformLocation(m_RendererID, name.c_str()) );

        if (location == -1) {
            std::cout << "Warning: uniform '" << name << "' doesn't exist - " << m_FilePath << '\n';
        }

        m_UniformLocationCache[name] = location;

        return location;
    }

    ShaderProgramSource Shader::ParseShader(const std::string& file_path)
    {
        std::ifstream stream(file_path);
        
        enum ShaderType { NONE = -1, VERTEX = 0, FRAGMENT = 1 };

        ShaderType type = ShaderType::NONE;

        std::string line;
        std::stringstream ss[2];

        while (getline(stream, line))
        {
            if (line.find("#shader") != std::string::npos) 
            {
                if (line.find("vertex") != std::string::npos) {
                    type = ShaderType::VERTEX;
                } else if (line.find("fragment") != std::string::npos) {
                    type = ShaderType::FRAGMENT;
                }
            }
            else
            {
                ss[(int)type] << line << '\n';
            }
        }

        return { ss[VERTEX].str(), ss[FRAGMENT].str() };
    }

    ShaderProgramSource Shader::ParseShader(const std::string& vertex_filepath, const std::string& fragment_filepath)
    {
        std::ifstream vert_ifstream(vertex_filepath);
        std::ifstream frag_ifstream(fragment_filepath);

        std::stringstream vert_sstream;
        std::stringstream frag_sstream;

        std::string line;

        while (getline(vert_ifstream, line))
            vert_sstream << line << '\n';

        while (getline(frag_ifstream, line))
            frag_sstream << line << '\n';

        return { vert_sstream.str(), frag_sstream.str() };
    }

    unsigned int Shader::CompileShader(unsigned int type, const std::string& source)
    {
        unsigned int id = glCreateShader(type);
        const char *src = source.c_str();
        glShaderSource(id, 1, &src, nullptr);
        glCompileShader(id);

        int result;
        glGetShaderiv(id, GL_COMPILE_STATUS, &result);

        if (result == GL_FALSE)
        {
            LOG("Compilation error: " + m_FilePath);

            int lenght;
            glGetShaderiv(id, GL_INFO_LOG_LENGTH, &lenght);

            char *message = new char[lenght];

            glGetShaderInfoLog(id, lenght, &lenght, message);
            LOG(message); 

            delete[] message;

            GLCall( glDeleteShader(id) );
            return 0;
        }

        return id;
    }

    unsigned int Shader::CreateShader(const std::string& vertex_shader, const std::string& fragment_shader)
    {
        GLCall( unsigned int program = glCreateProgram() );

        unsigned int vs = CompileShader(GL_VERTEX_SHADER, vertex_shader);
        unsigned int fs = CompileShader(GL_FRAGMENT_SHADER, fragment_shader);

        GLCall( glAttachShader(program, vs) );
        GLCall( glAttachShader(program, fs) );
        GLCall( glLinkProgram(program) );
        GLCall( glValidateProgram(program) );

        GLCall( glDeleteShader(vs) );
        GLCall( glDeleteShader(fs) );

        return program;
    }
}