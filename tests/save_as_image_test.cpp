#include "../src/camera.hpp"
#include "../src/layer_control.hpp"
#include "../src/layer.hpp"
#include "../src/project.hpp"
#include "../src/tool.hpp"
#include "../src/gl_context.hpp"
#include "../src/color.hpp"

#include "../src/gla/pixel_buffer.hpp"

#include <GLFW/glfw3.h>

#include <cstddef>
#include <print>
#include <span>
#include <string>
#include <vector>
#include <iostream>
#include <exception>

auto main(int argc, const char* argv[]) -> int {
    try {
        auto args = std::span(argv, static_cast<std::size_t>(argc));

        Gla::GLContext gl_context;
        GLFWwindow* window = gl_context.InitOpenGL(args);

        std::string full_path = args[1];
        if (!full_path.ends_with(".png")) {
            full_path += "/output.png";
        }

        const Pikzel::Vec2 project_dims{1000, 1000};
        std::vector<Gla::Color> simulated_pbo_data{
            static_cast<std::size_t>(project_dims.x * project_dims.y),
            Gla::Color{.r = 0, .g = 0, .b = 0, .a = 0}};
        const Gla::PboMappedBuffSpan span{simulated_pbo_data};

        Pikzel::LayerControl layers;
        Pikzel::Tool tool;
        Pikzel::Camera camera;
        Pikzel::Project project{layers, tool, camera};
        std::println("Making new project...");
        project.New(project_dims);
        std::println("Made new project.");
        layers.UpdateMappedPBOMemorySpanForAllLayers(span);

        layers.AddLayer(tool, camera);

        constexpr int kOpacity = 100;
        constexpr Pikzel::Color kDrawCol1{.r = 199, .g = 19, .b = 66, .a = 255};

        layers.GetCurrentLayer().DrawCircle(
            project_dims / 2, project_dims.x / 2,
            Pikzel::Layer::DrawType::kFill,
            Pikzel::ColorConstants::kColorTransparent, kDrawCol1);
        layers.GetCurrentLayer().SetOpacity(kOpacity);

        constexpr Pikzel::Color kDrawCol2{
            .r = 18, .g = 180, .b = 100, .a = 255};

        layers.SetCurrentLayer(1);

        layers.GetCurrentLayer().DrawCircle(
            project_dims / 2, project_dims.x / 2,
            Pikzel::Layer::DrawType::kFill,
            Pikzel::Color{.r = 0, .g = 0, .b = 0, .a = 0}, kDrawCol2);
        layers.GetCurrentLayer().SetOpacity(kOpacity);

        if (!project.SaveAsImage(1, full_path)) {
            std::println("An error occured during saving.");
            glfwDestroyWindow(window);
            glfwTerminate();
            return 1;
        }

        std::println("Should have saved successfuly");
    } catch (const std::exception& e) {
        std::cerr << "Unhandled exception: " << e.what() << '\n';
        return 1;
    } catch (...) {
        std::cerr << "Unknown unhandled exception\n";
        return 1;
    }
}
