#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#include "gla/frame_buffer.hpp"
#include "gla/group.hpp"
#include "gla/renderer.hpp"
#include "gla/timer.hpp"
#include "gla/vertex_array.hpp"
#include "gla/vertex_buffer.hpp"
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "application.hpp"
#include "events.hpp"
#include "layer.hpp"
#include "layer_control.hpp"
#include "preview_layer.hpp"
#include "project.hpp"
#include "vertex_buffer_control.hpp"

#include <cmath>
#include <cstddef>
#include <future>
#include <iostream>
#include <string>
#include <vector>

using Pikzel::Vertex;

namespace
{
constexpr int kWindowWidth = 1280;
constexpr int kWindowHeight = 700;

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
    std::cerr << "OpenGL Debug Message:" << "\n  Source: " << source_str
              << "\n  Type: " << type_str << "\n  Severity: " << severity_str
              << "\n  ID: " << errorId << "\n  Message: " << message << '\n';
}

void GlfwError(int err_id, const char* message)
{
    std::cerr << "Error id: " << err_id << "\nError message: " << message
              << '\n';
}
#endif

auto GetProjMat(Pikzel::Camera& camera, Pikzel::Vec2Int canvas_dims)
    -> glm::mat4
{
    const auto width = static_cast<float>(canvas_dims.x);
    const auto height = static_cast<float>(canvas_dims.y);
    const glm::vec2 camera_top_left =
        camera.GetCenterAsVec2Int() - canvas_dims / 2;
    auto zoom_half = static_cast<float>(camera.GetZoomValue()) / 2;

    glm::mat4 proj =
        glm::ortho((zoom_half * width) + camera_top_left.x,
                   width - (zoom_half * width) + camera_top_left.x,

                   (zoom_half * height) + camera_top_left.y,
                   height - (zoom_half * height) + camera_top_left.y);

    return proj;
}

auto GetTransMat(Pikzel::Vec2Int canvas_coord_behind_cursor,
                 Pikzel::Vec2Int canvas_dims) -> glm::mat4
{
    glm::mat4 translation_mat(1.0);
    auto move_distance = canvas_coord_behind_cursor - (canvas_dims / 2);

    translation_mat = glm::translate(
        glm::mat4(1.0), glm::vec3(move_distance.x, move_distance.y, 1.0));

    return translation_mat;
}

// Binds the vbo
void UpdatePreviewVboIfNeeded(Pikzel::PreviewLayer& preview_layer,
                              std::vector<Vertex>& preview_vertices,
                              Gla::VertexBuffer& vbo_preview)
{
    vbo_preview.Bind();
    if (preview_layer.IsPreviewLayerChanged())
    {
        preview_vertices.clear();
        preview_layer.EmplaceVertices(preview_vertices);
        vbo_preview.UpdateSizeIfNeeded(preview_vertices.size() *
                                       sizeof(Vertex));
        vbo_preview.UpdateData(preview_vertices.data(),
                               preview_vertices.size() * sizeof(Vertex));
    }
}

auto ImVec2Equal(ImVec2 vec_a, ImVec2 vec_b) -> bool
{
    constexpr float kAllowedDiff = 0.01F;

    return (std::abs(vec_a.x - vec_b.x) <= kAllowedDiff &&
            std::abs(vec_a.y - vec_b.y) <= kAllowedDiff);
}

void MainLoop(GLFWwindow* window)
{
    Pikzel::Tool tool;
    Pikzel::Camera camera;
    Pikzel::Layers layers;
    Pikzel::Project project{layers, tool, camera};
    Pikzel::UI ui_state{project, tool, window};

    Pikzel::Events::PushToScrollCallback(
        [&camera, &ui_state](double x_offset, double y_offset)
        {
            if (ui_state.ShouldDoTool())
            {
                camera.ScrollCallback(x_offset, y_offset);
            }
        });
    Pikzel::Events::PushToCursorPosCallback(
        [&camera, &ui_state](double x_offset, double y_offset)
        {
            if (ui_state.ShouldDoTool())
            {
                camera.CursorPosCallback(x_offset, y_offset);
            }
        });

    Gla::FrameBuffer imgui_window_fb(
        {.width = kWindowWidth, .height = kWindowHeight});
    Gla::FrameBuffer::BindToDefaultFB();

    Gla::Texture2D brush_tool_texture("assets/brush_tool.png",
                                      Gla::GLMinMagFilter::kNearest, true);
    Gla::Texture2D eraser_tool_texture("assets/eraser_tool.png",
                                       Gla::GLMinMagFilter::kNearest);
    Gla::Texture2D color_picker_tool_texture("assets/color_picker_tool.png",
                                             Gla::GLMinMagFilter::kNearest);
    Gla::Texture2D bucket_tool_texture("assets/bucket_tool.png",
                                       Gla::GLMinMagFilter::kNearest);
    Gla::Texture2D square_tool_texture("assets/square_tool.png");

    std::array<unsigned int,
               static_cast<std::size_t>(Pikzel::ToolType::kToolCount)>
        tool_texture_ids = {
            brush_tool_texture.GetID(),        eraser_tool_texture.GetID(),
            color_picker_tool_texture.GetID(), bucket_tool_texture.GetID(),
            square_tool_texture.GetID(),
        };

    Gla::Texture2D eye_opened_texture("assets/eye_opened.png");
    Gla::Texture2D eye_closed_texture("assets/eye_closed.png");
    Gla::Texture2D lock_locked_texture("assets/lock_locked.png");
    Gla::Texture2D lock_unlocked_texture("assets/lock_unlocked.png");

    std::array<unsigned int, 4> layer_texture_ids = {
        eye_opened_texture.GetID(), eye_closed_texture.GetID(),
        lock_locked_texture.GetID(), lock_unlocked_texture.GetID()};

    ui_state.SetupToolTextures(tool_texture_ids);
    ui_state.SetupLayerToolTextures(layer_texture_ids);

    Gla::VertexBufferLayout layout;
    layout.Push<float>(2);
    layout.Push<uint8_t>(4, GL_TRUE);
    Gla::Shader shader("shader/vert_shader.vert", "shader/frag_shader.frag");
    shader.Bind();

    Gla::VertexArray vao_canvas;
    Gla::VertexBuffer vbo_canvas(nullptr, 0, Gla::kDynamicDraw);
    vao_canvas.AddBuffer(vbo_canvas, layout);
    Gla::Group group_canvas(vao_canvas, shader);

    Gla::VertexArray vao_bckg;
    Gla::VertexBuffer vbo_bckg(nullptr, 0, Gla::kStaticDraw);
    vao_bckg.AddBuffer(vbo_bckg, layout);
    Gla::Shader shader_bckg("shader/background_vert_shader.vert",
                            "shader/background_frag_shader.frag");
    shader_bckg.Bind();
    Gla::Group group_bckg(vao_bckg, shader_bckg);
    auto bckg_vertices_count = 0UZ;

    Gla::VertexArray vao_preview;
    Gla::VertexBuffer vbo_preview(nullptr, 0, Gla::kDynamicDraw);
    vao_preview.AddBuffer(vbo_preview, layout);
    Gla::Shader shader_preview("shader/vert_shader.vert",
                               "shader/frag_shader.frag");
    Gla::Group group_preview(vao_preview, shader_preview);
    std::vector<Vertex> preview_vertices;

    std::optional<Pikzel::PreviewLayer> preview_layer;
    std::optional<Pikzel::VertexBufferControl> vbo_control;
    std::future<void> vbo_update_future;
    ImVec2 draw_window_dims;
    float prev_fps = 0.0F;
    Gla::Timer out_of_loop_timer;

    while (glfwWindowShouldClose(window) == 0)
    {
#ifndef NDEBUG
        Gla::Timer timer;
#endif

        Pikzel::UI::NewFrame();

        if (project.IsOpened())
        {
            ui_state.RenderUI(layers, camera);
            ui_state.RenderDrawWindow(imgui_window_fb.GetTextureID(), "Draw");
        }
        else
        {
            ui_state.RenderNoProjectWindow();

            if (project.IsOpened())
            {
                std::vector<Vertex> bckg_vertices;
                layers.EmplaceBckgVertices(bckg_vertices,
                                           project.GetCanvasDims());
                bckg_vertices_count = bckg_vertices.size();
                auto bckg_buff_size = bckg_vertices.size() * sizeof(Vertex);
                vbo_bckg.UpdateSize(bckg_buff_size);
                vbo_bckg.UpdateData(bckg_vertices.data(), bckg_buff_size);

                std::size_t vertex_count =
                    static_cast<std::size_t>(project.CanvasWidth() *
                                             project.CanvasHeight()) *
                    Pikzel::kVerticesPerPixel;

                vbo_canvas.UpdateSize(vertex_count * sizeof(Vertex));

                auto* buff_data = static_cast<Vertex*>(
                    glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY));

                vbo_control.emplace(layers, buff_data, vertex_count);
                preview_layer.emplace(tool, camera, layers.GetCanvasDims());
            }
        }

        Pikzel::UI::RenderAndEndFrame();

        if (project.IsOpened() && ui_state.IsDrawWindowRendered())
        {
            assert(preview_layer.has_value());
            assert(vbo_control.has_value());

            auto proj_mat = GetProjMat(camera, project.GetCanvasDims());
            shader.Bind();
            shader.SetUniformMat4f("u_ViewProjection", proj_mat);

            imgui_window_fb.Bind();

            if (!ImVec2Equal(draw_window_dims, ui_state.GetDrawWinDimensions()))
            {
                ImVec2 draw_window_dims = ui_state.GetDrawWinDimensions();
                imgui_window_fb.Rescale(
                    {.width = static_cast<int>(draw_window_dims.x),
                     .height = static_cast<int>(draw_window_dims.y)});
            }

            Gla::Renderer::Clear();
            glClearColor(0.8, 0.8, 0.8, 1.0);

            group_bckg.Bind();
            shader_bckg.SetUniformMat4f(
                "u_ViewProjection",
                GetProjMat(camera, project.GetCanvasDims()));
            Gla::Renderer::DrawArrays(Gla::kTriangles, bckg_vertices_count);

            if (vbo_update_future.valid()) { vbo_update_future.wait(); }

            group_canvas.Bind();
            vbo_control.value().UpdateSizeIfNeeded(vbo_canvas);

            Pikzel::VertexBufferControl::Unmap(vbo_canvas);
            Gla::Renderer::DrawArrays(Gla::kTriangles,
                                      vbo_control->GetVertexCount());
            vbo_control->Map(vbo_canvas);

            layers.UpdateAndDraw(ui_state.ShouldDoTool(), tool, camera);
            vbo_update_future = std::async(
                std::launch::async,
                [&vbo_control, &prev_fps]()
                {
                    Gla::Timer timer;
                    vbo_control->Update(Pikzel::Layer::ShouldUpdateWholeVBO(),
                                        Pikzel::Layer::GetDirtyPixels());
                    Pikzel::Layer::ResetDirtyPixelData();
                    prev_fps = 1 / timer.GetTime();
                });

            UpdatePreviewVboIfNeeded(preview_layer.value(), preview_vertices,
                                     vbo_preview);
            auto canvas_coord_behind_cursor =
                layers.CanvasCoordsFromCursorPos();
            if (canvas_coord_behind_cursor.has_value() &&
                ui_state.ShouldDoTool())
            {
                glm::mat4 trans_mat = GetTransMat(
                    canvas_coord_behind_cursor.value(), layers.GetCanvasDims());
                group_preview.Bind();
                glm::mat4 result = proj_mat;

                if (preview_layer->ShouldApplyCursorBasedTranslation())
                {
                    result *= trans_mat;
                }

                shader_preview.SetUniformMat4f("u_ViewProjection", result);
                Gla::Renderer::DrawArrays(Gla::kTriangles,
                                          preview_vertices.size());
            }

            Gla::FrameBuffer::BindToDefaultFB();

            ui_state.Update();
            preview_layer->Update();
        }

        Pikzel::Events::Update();

        glfwSwapBuffers(window);

        ui_state.SetShouldDoToolToTrue();

#ifndef NDEBUG
        float fps = 1.0F / timer.GetTime();
        if (out_of_loop_timer.GetTime() > 0.2)
        {
            std::string win_title =
                "Pikzel - FPS: " + std::to_string(fps) +
                " /// PREV_FPS: " + std::to_string(prev_fps);
            glfwSetWindowTitle(window, win_title.c_str());
            out_of_loop_timer.Reset();
        }
#endif
    }
}
} // namespace

auto main(int argc, const char* argv[]) -> int
{
    if (glfwInit() == GLFW_FALSE) { return 1; }

    GLFWwindow* window = glfwCreateWindow(kWindowWidth, kWindowHeight, "Pikzel",
                                          nullptr, nullptr);

    if (window == nullptr)
    {
        std::cout << "Failed to create window\n";
        glfwTerminate();
        return 1;
    }

    glfwMakeContextCurrent(window);

    // NOLINTNEXTLINE
    if (argc > 1 && argv[1] == std::string{"no_vsync"}) { glfwSwapInterval(0); }
    else { glfwSwapInterval(1); }

    glfwMaximizeWindow(window);

    Pikzel::Events::SetWindowPtr(window);
    glfwSetScrollCallback(window, &Pikzel::Events::GlfwScrollCallback);
    glfwSetCursorPosCallback(window, &Pikzel::Events::GlfwCursorPosCallback);

#ifndef NDEBUG
    glfwSetErrorCallback(&GlfwError);
    std::cout << "C++ standard: " << __cplusplus << '\n';
#endif

    if (glewInit() != GLEW_OK)
    {
        std::cout << "Glew init error\n";
        std::cout << glewGetErrorString(glewInit()) << '\n';
        return 1;
    }

#ifndef NDEBUG
    std::cout << "OpenGL version: " << glGetString(GL_VERSION) << '\n';
    glEnable(GL_DEBUG_OUTPUT);
    glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
    glDebugMessageCallback(GlDebugOutput, nullptr);
#endif

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    MainLoop(window);

    glfwDestroyWindow(window);
    glfwTerminate();
}
