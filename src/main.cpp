#include <glad/gl.h>
#include <GLFW/glfw3.h>

#include <cstddef>
#include <print>
#include <span>
#include <iostream>
#include <string>

#include "app.hpp"
#include "events.hpp"

namespace {
auto InitOpenGL(std::span<const char*> args) -> GLFWwindow*;
void DeinitOpenGL(GLFWwindow* window);
#ifndef NDEBUG
void GLAPIENTRY GlDebugOutput(GLenum source, GLenum type, GLuint errorId,
                              GLenum severity, GLsizei /*length*/,
                              const GLchar* message, const void* /*userParam*/);
void GlfwError(int err_id, const char* message);
#endif
} // namespace

auto main(int argc, const char* argv[]) -> int {
    const std::span args{argv, static_cast<std::size_t>(argc)};
    GLFWwindow* window = InitOpenGL(args);

    if (window == nullptr) {
        std::println(std::cerr, "Failed to load OpenGL. Aborting...");
        return 1;
    }

    Pikzel::App app{window};
    app.MainLoop();

    DeinitOpenGL(window);
}

namespace {
auto InitOpenGL(std::span<const char*> args) -> GLFWwindow* {
    if (glfwInit() == GLFW_FALSE) {
        return nullptr;
    }

    glfwWindowHint(GLFW_SCALE_TO_MONITOR, GLFW_TRUE);
    GLFWwindow* window = glfwCreateWindow(640, 480, "Pikzel", nullptr, nullptr);

    if (window == nullptr) {
        std::println("Failed to create window");
        glfwTerminate();
        return nullptr;
    }

    glfwMakeContextCurrent(window);

    if (args.size() > 1 && args[1] == std::string{"no_vsync"}) {
        glfwSwapInterval(0);
    } else {
        glfwSwapInterval(1);
    }

    glfwMaximizeWindow(window);

#ifndef NDEBUG
    glfwSetErrorCallback(&GlfwError);
    std::println("C++ standard: {}", __cplusplus);
#endif

    const int version = gladLoadGL(glfwGetProcAddress);

    if (version == 0) {
        std::println("gladLoadGL: Failed to load GL.");
        return nullptr;
    }

#ifndef NDEBUG
    std::cout << glGetString(GL_VERSION) << '\n';
    glEnable(GL_DEBUG_OUTPUT);
    glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
    glDebugMessageCallback(GlDebugOutput, nullptr);
#endif

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Have to do this before initializing ImGui, since this code breaks ImGui
    // after its initialization
    Pikzel::Events::SetWindowPtr(window);
    glfwSetScrollCallback(window, &Pikzel::Events::GlfwScrollCallback);
    glfwSetCursorPosCallback(window, &Pikzel::Events::GlfwCursorPosCallback);

    return window;
}

void DeinitOpenGL(GLFWwindow* window) {
    glfwDestroyWindow(window);
    glfwTerminate();
}

#ifndef NDEBUG
void GLAPIENTRY GlDebugOutput(GLenum source, GLenum type, GLuint errorId,
                              GLenum severity, GLsizei /*length*/,
                              const GLchar* message,
                              const void* /*userParam*/) {
    std::string source_str = "[Unknown]";
    switch (source) {
    case GL_DEBUG_SOURCE_API:
        source_str = "API";
        break;
    case GL_DEBUG_SOURCE_WINDOW_SYSTEM:
        source_str = "Window System";
        break;
    case GL_DEBUG_SOURCE_SHADER_COMPILER:
        source_str = "Shader Compiler";
        break;
    case GL_DEBUG_SOURCE_THIRD_PARTY:
        source_str = "Third Party";
        break;
    case GL_DEBUG_SOURCE_APPLICATION:
        source_str = "Application";
        break;
    case GL_DEBUG_SOURCE_OTHER:
        source_str = "Other";
        break;
    default:
        break;
    }

    std::string type_str = "[Unknown]";
    switch (type) {
    case GL_DEBUG_TYPE_ERROR:
        type_str = "Error";
        break;
    case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR:
        type_str = "Deprecated Behavior";
        break;
    case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:
        type_str = "Undefined Behavior";
        break;
    case GL_DEBUG_TYPE_PORTABILITY:
        type_str = "Portability";
        break;
    case GL_DEBUG_TYPE_PERFORMANCE:
        type_str = "Performance";
        break;
    case GL_DEBUG_TYPE_MARKER:
        type_str = "Marker";
        break;
    case GL_DEBUG_TYPE_PUSH_GROUP:
        type_str = "Push Group";
        break;
    case GL_DEBUG_TYPE_POP_GROUP:
        type_str = "Pop Group";
        break;
    case GL_DEBUG_TYPE_OTHER:
        type_str = "Other";
        break;
    default:
        break;
    }

    std::string severity_str = "[Unknown]";
    switch (severity) {
    case GL_DEBUG_SEVERITY_HIGH:
        severity_str = "High";
        break;
    case GL_DEBUG_SEVERITY_MEDIUM:
        severity_str = "Medium";
        break;
    case GL_DEBUG_SEVERITY_LOW:
        severity_str = "Low";
        break;
    case GL_DEBUG_SEVERITY_NOTIFICATION:
        severity_str = "Notification";
        break;
    default:
        break;
    }

    static int error_count = 0;

    std::cerr << "OpenGL Debug Message:" << "\n  Source: " << source_str
              << "\n  Type: " << type_str << "\n  Severity: " << severity_str
              << "\n  ID: " << errorId << "\n  Message: " << message << '\n'
              << "Errors printed count: " << error_count << '\n';

    error_count++;
}

void GlfwError(int err_id, const char* message) {
    std::println(std::cerr, "Glfw error id: {}\nError message: {}", err_id,
                 message);
}
#endif
} // namespace
