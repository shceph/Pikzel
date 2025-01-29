#include "GlaBase.hpp"

auto GLErrorToString(GLenum error_code) -> const char*
{
    switch (error_code)
    {
    case GL_INVALID_ENUM:
        return "INVALID_ENUM";
    case GL_INVALID_VALUE:
        return "INVALID_VALUE";
    case GL_INVALID_OPERATION:
        return "INVALID_OPERATION";
    case GL_STACK_OVERFLOW:
        return "STACK_OVERFLOW";
    case GL_STACK_UNDERFLOW:
        return "STACK_UNDERFLOW";
    case GL_OUT_OF_MEMORY:
        return "OUT_OF_MEMORY";
    case GL_INVALID_FRAMEBUFFER_OPERATION:
        return "INVALID_FRAMEBUFFER_OPERATION";
    default:
        return "couldn't recognize the following error code: 0x";
    }
}

void GLClearError()
{
    while (glGetError() != GL_NO_ERROR) {}
}

auto GLLogCall(const char* function_name, const char* file_name,
               int line) -> bool
{
    while (GLenum error = glGetError())
    {
        std::cout << "GL error: " << GLErrorToString(error) << '\n'
                  << "Function: " << function_name << '\n'
                  << "File: " << file_name << '\n'
                  << "Line: " << std::dec << line << '\n';

        return false;
    }

    return true;
}
