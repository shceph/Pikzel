#include "app.hpp"
#include "events.hpp"
#include "gl_context.hpp"

#include <GLFW/glfw3.h>

#include <cstddef>
#include <exception>
#include <iostream>
#include <print>
#include <span>

auto main(int argc, const char* argv[]) -> int {
    try {
        const std::span args{argv, static_cast<std::size_t>(argc)};

        Gla::GLContext gl_context;
        GLFWwindow* window = gl_context.InitOpenGL(args);

        if (window == nullptr) {
            std::println(std::cerr, "Failed to load OpenGL. Aborting...");
            return 1;
        }

        // Have to do this before initializing ImGui, since this code breaks
        // ImGui after its initialization
        Pikzel::Events::SetWindowPtr(window);
        glfwSetScrollCallback(window, &Pikzel::Events::GlfwScrollCallback);
        glfwSetCursorPosCallback(window,
                                 &Pikzel::Events::GlfwCursorPosCallback);

        Pikzel::App app{window};
        app.MainLoop();
    } catch (const std::exception& e) {
        std::cerr << "Unhandled exception: " << e.what() << '\n';
        return 1;
    } catch (...) {
        std::cerr << "Unknown unhandled exception\n";
        return 1;
    }
}
