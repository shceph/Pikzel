#pragma once

#define GLEW_STATIC

#include "GL/glew.h"
#include "GLFW/glfw3.h"
#include <iostream>

#define GLA_DEBUG

#define GLAssert(expression)    if (!expression) { std::exit(EXIT_FAILURE); }

#ifdef GLA_DEBUG
    #define LOG(output)  std::cout << output << '\n';

    #define GLCall(expression)  GLClearError();\
                                expression;\
                                GLAssert(GLLogCall(#expression, __FILE__, __LINE__))
#else
    #define LOG(output)         { }
    #define GLCall(expression)  expression
#endif  // GLA_DEBUG

const char* GLErrorToString(GLenum error);
void        GLClearError();
bool        GLLogCall(const char *function_name, const char *file_name, int line);