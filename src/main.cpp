#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#include "Gla/FrameBuffer.hpp"
#include "Gla/Mesh.hpp"
#include "Gla/Renderer.hpp"
#include "Gla/VertexArray.hpp"
#include "Gla/VertexBuffer.hpp"
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <chrono>
#include <iostream>
#include <string>
#include <vector>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "Application.hpp"
#include "Layer.hpp"

namespace
{

#ifndef NDEBUG
// NOLINTNEXTLINE(bugprone-easily-swappable-parameters)
void GLAPIENTRY GlDebugOutput(GLenum source, GLenum type, GLuint errorId,
                              GLenum severity, GLsizei /*length*/,
                              const GLchar* message, const void* /*userParam*/)
{
    // Convert GLenum source, type, severity to strings for better readability
    std::string source_str = "[Unknown]";
    switch (source)
    {
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
    switch (type)
    {
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
    switch (severity)
    {
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

    // Output the debug message along with file and line information
    std::cerr << "OpenGL Debug Message:"
              << "\n  Source: " << source_str << "\n  Type: " << type_str
              << "\n  Severity: " << severity_str << "\n  ID: " << errorId
              << "\n  Message: " << message << '\n';
}

void GlfwError(int err_id, const char* message)
{
    std::cerr << "Error id: " << err_id << "\nError message: " << message
              << '\n';
}
#endif

void GlfwScrollCallback(GLFWwindow* /*window*/, double /*xoffset*/,
                        double yoffset)
{
    App::Camera::AddToZoom(yoffset / 30);
}

void GlfwCursorPosCallback(GLFWwindow* window, double x_pos, double y_pos)
{
    constexpr auto kAllowedDelay = std::chrono::milliseconds(25);
    static double old_x = x_pos;
    static double old_y = y_pos;
    static auto last_time_clicked = std::chrono::steady_clock::now();

    if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) ==
            GLFW_PRESS &&
        std::chrono::steady_clock::now() - last_time_clicked <= kAllowedDelay)
    {
        double offset_x = old_x - x_pos;
        double offset_y = old_y - y_pos;
        App::Camera::MoveCenter(
            {static_cast<int>(offset_x), static_cast<int>(offset_y)});
    }

    old_x = x_pos;
    old_y = y_pos;
    last_time_clicked = std::chrono::steady_clock::now();
}

void HandleEvents(GLFWwindow* window)
{
    glfwWaitEvents();
    /* glfwPollEvents(); */

    if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS &&
        App::Project::IsOpened())
    {
        App::Layers::DoCurrentTool();
    }
}

void UpdateProjMat(Gla::Shader& shader)
{
    assert(App::Project::IsOpened());

    const auto width = static_cast<float>(App::Project::CanvasWidth());
    const auto height = static_cast<float>(App::Project::CanvasHeight());
    const App::Vec2Int center_offset =
        App::Camera::GetCenter() - App::Project::GetCanvasDims() / 2;
    auto zoom_half = static_cast<float>(App::Camera::GetZoomValue()) / 2;

    glm::mat4 proj = glm::ortho(
        static_cast<float>(center_offset.x) + zoom_half * width,
        width - zoom_half * width + static_cast<float>(center_offset.x),
        static_cast<float>(center_offset.y) + zoom_half * height,
        height - zoom_half * height + static_cast<float>(center_offset.y));

    shader.SetUniformMat4f("u_ViewProjection", proj);
}
} // namespace

auto main() -> int
{
    if (glfwInit() == 0) { return -1; }

    // glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    // glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    // glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    constexpr int kWindowWidth = 1280;
    constexpr int kWindowHeight = 700;

    /* Create a windowed mode window and its OpenGL context */
    GLFWwindow* window = glfwCreateWindow(kWindowWidth, kWindowHeight,
                                          "PixelCraft", nullptr, nullptr);

    if (window == nullptr)
    {
        std::cout << "Failed to create window\n";
        glfwTerminate();
        std::cin.get();
        return -1;
    }

    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);
    glfwMaximizeWindow(window);

    glfwSetScrollCallback(window, &GlfwScrollCallback);
    glfwSetCursorPosCallback(window, &GlfwCursorPosCallback);
#ifndef NDEBUG
    glfwSetErrorCallback(&GlfwError);
    std::cout << "C++ standard: " << __cplusplus << '\n';
#endif

    if (glewInit() != GLEW_OK) { std::cout << "Glew init error\n"; }

    std::cout << "OpenGL version: " << glGetString(GL_VERSION) << '\n';

#ifndef NDEBUG
    glEnable(GL_DEBUG_OUTPUT);
    glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
    glDebugMessageCallback(GlDebugOutput, nullptr);
#endif

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_GREATER);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    App::UI::ImGuiInit(window);

    { // Made this block so all the OpenGL objects would get destroyed before
      // calling glfwTerminate.

        std::vector<App::Vertex> vertices;

        Gla::FrameBuffer imgui_window_fb(kWindowWidth, kWindowHeight);
        Gla::FrameBuffer::BindToDefaultFB();

        Gla::Renderer renderer;

        Gla::Texture2D brush_tool_texture("assets/brush_tool.png",
                                          Gla::GLMinMagFilter::NEAREST, true);
        Gla::Texture2D eraser_tool_texture("assets/eraser_tool.png",
                                           Gla::GLMinMagFilter::NEAREST);
        Gla::Texture2D color_picker_tool_texture("assets/color_picker_tool.png",
                                                 Gla::GLMinMagFilter::NEAREST);
        Gla::Texture2D bucket_tool_texture("assets/bucket_tool.png",
                                           Gla::GLMinMagFilter::NEAREST);

        Gla::Texture2D eye_opened_texture("assets/eye_opened.png",
                                          Gla::GLMinMagFilter::LINEAR);
        Gla::Texture2D eye_closed_texture("assets/eye_closed.png",
                                          Gla::GLMinMagFilter::LINEAR);
        Gla::Texture2D lock_locked_texture("assets/lock_locked.png",
                                           Gla::GLMinMagFilter::LINEAR);
        Gla::Texture2D lock_unlocked_texture("assets/lock_unlocked.png",
                                             Gla::GLMinMagFilter::LINEAR);

        App::UI::SetupToolTextures(
            brush_tool_texture.GetID(), eraser_tool_texture.GetID(),
            color_picker_tool_texture.GetID(), bucket_tool_texture.GetID());

        App::UI::SetupLayerToolTextures(
            eye_opened_texture.GetID(), eye_closed_texture.GetID(),
            lock_locked_texture.GetID(), lock_unlocked_texture.GetID());

        Gla::VertexArray vao;
        Gla::VertexBuffer vbo(nullptr, 0);
        Gla::VertexBufferLayout layout;
        layout.Push<float>(2);
        layout.Push<uint8_t>(4, GL_TRUE);
        vao.AddBuffer(vbo, layout);
        Gla::Shader shader("shader/VertShader.vert", "shader/FragShader.frag");
        shader.Bind();
        Gla::Mesh mesh(vao, shader);
        mesh.Bind();

        /* Loop until the user closes the window */
        while (glfwWindowShouldClose(window) == 0)
        {
            App::UI::NewFrame();

            if (App::Project::IsOpened())
            {
                App::UI::RenderUI();
                App::UI::RenderDrawWindow(imgui_window_fb.GetTextureID(),
                                          "Draw");
            }
            else
            {
                App::UI::RenderNoProjectWindow();

                if (App::Project::IsOpened()) // If a new project got created
                {
                    glm::mat4 proj = glm::ortho(
                        0.0F, static_cast<float>(App::Project::CanvasWidth()),
                        0.0F, static_cast<float>(App::Project::CanvasHeight()));
                    shader.SetUniformMat4f("u_ViewProjection", proj);
                    // Setting up the vertex vector for the first time
                    App::UI::SetVertexBuffUpdateToTrue();
                }
            }

            App::UI::RenderAndEndFrame();

            if (App::Project::IsOpened() && App::UI::IsDrawWindowRendered())
            {
                App::Layers::DrawToTempLayer();
                App::UI::SetVertexBuffUpdateToTrue();
            }

            if (App::UI::ShouldUpdateVertexBuffer())
            {
                mesh.Bind();
                App::Layers::EmplaceVertices(vertices);
                vbo.UpdateSizeIfNeeded(vertices.size() * sizeof(App::Vertex));
                vbo.UpdateData(vertices.data(),
                               vertices.size() * sizeof(App::Vertex));
            }

            if (App::Project::IsOpened() && App::UI::IsDrawWindowRendered())
            {
                ImVec2 draw_window_dims = App::UI::GetDrawWinDimensions();
                imgui_window_fb.Bind();
                imgui_window_fb.Rescale(static_cast<int>(draw_window_dims.x),
                                        static_cast<int>(draw_window_dims.y));
                mesh.Bind();
                // TODO(scheph): Not to update the uniform each frame like this
                UpdateProjMat(shader);

                renderer.Clear();
                renderer.DrawArrays(Gla::TRIANGLES, vertices.size());

                Gla::FrameBuffer::BindToDefaultFB();
            }

            App::UI::Update();

            /* Swap front and back buffers */
            glfwSwapBuffers(window);

            HandleEvents(window);

            App::UI::SetShouldDoToolToTrue();
        }
    }

    App::UI::ImGuiCleanup();
    glfwDestroyWindow(window);
    glfwTerminate();
}
