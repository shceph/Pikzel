#include "../src/camera.hpp"
#include "../src/gla/pixel_buffer.hpp"
#include "../src/layer_control.hpp"
#include "../src/project.hpp"
#include "../src/tool.hpp"

#include <print>
#include <span>

auto main(int argc, char* argv[]) -> int
{
    auto args = std::span(argv, static_cast<std::size_t>(argc));

    if (glfwInit() == GLFW_FALSE)
    {
        std::println("Failed to initialize GLFW.");
        return 1;
    }

    glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
    GLFWwindow* window = glfwCreateWindow(1, 1, "", nullptr, nullptr);

    if (window == nullptr)
    {
        std::println("Failed to create window.");
        glfwTerminate();
        return 1;
    }

    int version = gladLoadGL(glfwGetProcAddress);

    if (version == 0)
    {
        std::println("gladLoadGL: Failed to load GL.");
        return 0;
    }

    if (argc < 2)
    {
        std::println("No path provided. Aborting.");
        glfwDestroyWindow(window);
        glfwTerminate();
        return 1;
    }

    std::string full_path = args[1];
    if (!full_path.ends_with(".png")) { full_path += "/output.png"; }

    Pikzel::Vec2Int project_dims{1000, 1000};
    std::vector<Gla::Color> simulated_pbo_data{
        static_cast<std::size_t>(project_dims.x * project_dims.y),
        Gla::Color{.r = 0, .g = 0, .b = 0, .a = 0}};
    Gla::PboMappedBuffSpan span{simulated_pbo_data};

    Pikzel::LayerControl layers;
    layers.UpdateMappedPBOMemorySpanForAllLayers(span);
    Pikzel::Tool tool;
    Pikzel::Camera camera;
    Pikzel::Project project{layers, tool, camera};
    std::println("Making new project...");
    project.New(project_dims);
    std::println("Made new project.");

    layers.AddLayer(tool, camera);

    layers.GetCurrentLayer().DrawCircle(
        project_dims / 2, project_dims.x / 2, Pikzel::Layer::DrawType::kFill,
        Pikzel::Color{.r = 0, .g = 0, .b = 0, .a = 0},
        Pikzel::Color{.r = 199, .g = 19, .b = 66, .a = 255});
    layers.GetCurrentLayer().SetOpacity(100);

    layers.SetCurrentLayer(1);

    layers.GetCurrentLayer().DrawCircle(
        project_dims / 2, project_dims.x / 2, Pikzel::Layer::DrawType::kFill,
        Pikzel::Color{.r = 0, .g = 0, .b = 0, .a = 0},
        Pikzel::Color{.r = 18, .g = 180, .b = 100, .a = 255});
    layers.GetCurrentLayer().SetOpacity(100);

    if (!project.SaveAsImage(1, full_path))
    {
        std::println("An error occured during saving.");
        glfwDestroyWindow(window);
        glfwTerminate();
        return 1;
    }

    std::println("Should have saved successfuly");

    glfwDestroyWindow(window);
    glfwTerminate();
}
