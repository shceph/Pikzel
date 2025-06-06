#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#include "gla/frame_buffer.hpp"
#include "gla/group.hpp"
#include "gla/pixel_buffer.hpp"
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

#include <cmath>
#include <future>
#include <print>
#include <string>

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

    static int error_count = 0;

    // Output the debug message along with file and line information
    std::cerr << "OpenGL Debug Message:" << "\n  Source: " << source_str
              << "\n  Type: " << type_str << "\n  Severity: " << severity_str
              << "\n  ID: " << errorId << "\n  Message: " << message << '\n'
              << "Errors printed count: " << error_count << '\n';

    error_count++;
}

void GlfwError(int err_id, const char* message)
{
    std::println(std::cerr, "Error id: {}\nError message: {}", err_id, message);
}
#endif

auto GetProjMat(const Pikzel::Camera& camera, Pikzel::Vec2Int canvas_dims)
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

auto ImVec2Equal(ImVec2 vec_a, ImVec2 vec_b) -> bool
{
    constexpr float kAllowedDiff = 0.01F;

    return (std::abs(vec_a.x - vec_b.x) <= kAllowedDiff &&
            std::abs(vec_a.y - vec_b.y) <= kAllowedDiff);
}

struct AppState
{
    explicit AppState(GLFWwindow* window, Gla::PboMappedBuffSpan& pbo_buff)
        : camera{}, layers{pbo_buff}, project{layers, tool, camera},
          ui_state{project, tool, window}, preview_layer{std::nullopt},
          preview_layer_for_selection{std::nullopt}
    {
    }

    Pikzel::Tool tool;
    Pikzel::Camera camera;
    Pikzel::Layers layers;
    Pikzel::Project project;
    Pikzel::UI ui_state;
    std::optional<Pikzel::PreviewLayer> preview_layer;
    std::optional<Pikzel::PreviewLayer> preview_layer_for_selection;
};

void UpdateVboBckg(Gla::VertexBuffer& vbo_bckg, Gla::Shader& shader_bckg,
                   glm::mat4 canvas_mat, Pikzel::Vec2Int canvas_dims)
{
    glm::vec2 top_left = canvas_mat * glm::vec4{0, 0, 0, 1};
    glm::vec2 bottom_right = canvas_mat * glm::vec4{canvas_dims, 0, 1};

    std::array<float, 8> bckg_vertices = {
        top_left.x,     top_left.y,

        bottom_right.x, top_left.y,

        top_left.x,     bottom_right.y,

        bottom_right.x, bottom_right.y,
    };

    vbo_bckg.UpdateData(bckg_vertices.data(), 8 * sizeof(float));

    glm::vec2 top_left_in_uv = (top_left + glm::vec2{1}) / glm::vec2{2};
    shader_bckg.SetUniform2f("u_TopLeftInUV", top_left_in_uv.x,
                             top_left_in_uv.y);
}

void HandleInputAndUI(
    AppState& app_state, Gla::FrameBuffer& imgui_window_fb,
    Gla::Shader& shader_bckg, Gla::VertexBuffer& vbo_canvas,
    Gla::PixelBuffer& pbo, Gla::PboMappedBuffSpan& pbo_buff,
    Gla::PixelBuffer& pbo_prev_layer,
    Gla::PboMappedBuffSpan& preview_layer_pbo_buff,
    Gla::PixelBuffer& pbo_prev_layer_for_selection,
    Gla::PboMappedBuffSpan& preview_layer_for_selection_pbo_buff)
{
    Pikzel::Events::Update();
    app_state.ui_state.SetShouldDoToolToTrue();

    Pikzel::UI::NewFrame();

    if (app_state.project.IsOpened())
    {
        assert(app_state.preview_layer_for_selection.has_value());
        app_state.ui_state.RenderUI(app_state.layers, app_state.camera,
                                    app_state.layers.GetSelection(),
                                    *app_state.preview_layer_for_selection);
        app_state.ui_state.RenderDrawWindow(imgui_window_fb.GetTextureID(),
                                            "Draw");
    }
    else
    {
        app_state.ui_state.RenderNoProjectWindow();

        if (app_state.project.IsOpened())
        {
            app_state.preview_layer.emplace(app_state.tool, app_state.camera,
                                            preview_layer_pbo_buff,
                                            app_state.layers.GetCanvasDims());
            app_state.preview_layer_for_selection.emplace(
                app_state.tool, app_state.camera,
                preview_layer_for_selection_pbo_buff,
                app_state.layers.GetCanvasDims());

            float height_over_width =
                static_cast<float>(app_state.project.CanvasHeight()) /
                static_cast<float>(app_state.project.CanvasWidth());
            shader_bckg.Bind();
            shader_bckg.SetUniform1f("u_CanvasHeightOverWidth",
                                     height_over_width);

            auto can_width =
                static_cast<float>(app_state.project.CanvasWidth());
            auto can_height =
                static_cast<float>(app_state.project.CanvasHeight());

            std::array<float, 24> vbo_canvas_vertices = {
                0.0F,      0.0F,       0.0F, 0.0F,

                can_width, 0.0F,       1.0F, 0.0F,

                0.0F,      can_height, 0.0F, 1.0F,

                can_width, 0.0F,       1.0F, 0.0F,

                0.0F,      can_height, 0.0F, 1.0F,

                can_width, can_height, 1.0F, 1.0F,
            };

            vbo_canvas.Bind();
            vbo_canvas.UpdateData(vbo_canvas_vertices.data(),
                                  vbo_canvas_vertices.size() * sizeof(float));
            Gla::VertexBuffer::Unbind();

            pbo.BindAndResize(app_state.project.GetCanvasDims(), Gla::Color{});
            pbo_buff = pbo.Map();
            Gla::PixelBuffer::Unbind();

            pbo_prev_layer.BindAndResize(app_state.project.GetCanvasDims(),
                                         Gla::Color{});
            preview_layer_pbo_buff = pbo_prev_layer.Map();
            Gla::PixelBuffer::Unbind();

            pbo_prev_layer_for_selection.BindAndResize(
                app_state.project.GetCanvasDims(), Gla::Color{});
            preview_layer_for_selection_pbo_buff =
                pbo_prev_layer_for_selection.Map();
            Gla::PixelBuffer::Unbind();
        }
    }

    Pikzel::UI::RenderAndEndFrame();
}

void Update(AppState& app_state)
{
    if (app_state.layers.HaveChosenDifferentLayerThisFrame())
    {
        app_state.layers.WriteCurrentLayerTextureDataToPbo();
    }

    app_state.preview_layer->Update();
    app_state.layers.UpdateAndDraw(
        app_state.ui_state.ShouldDoTool(), app_state.tool, app_state.camera,
        *app_state.preview_layer, *app_state.preview_layer_for_selection);
    app_state.ui_state.Update();
}

void Render(AppState& app_state, Gla::FrameBuffer& imgui_window_fb,
            Gla::Group& group_bckg, Gla::Shader& shader_bckg,
            Gla::VertexBuffer& vbo_bckg)
{
    static ImVec2 draw_window_dims;
    static std::future<void> vbo_update_future;

    auto proj_mat =
        GetProjMat(app_state.camera, app_state.project.GetCanvasDims());

    imgui_window_fb.Bind();

    if (!ImVec2Equal(draw_window_dims,
                     app_state.ui_state.GetDrawWinDimensions()))
    {
        ImVec2 draw_window_dims = app_state.ui_state.GetDrawWinDimensions();
        imgui_window_fb.Rescale(
            {.width = static_cast<int>(draw_window_dims.x),
             .height = static_cast<int>(draw_window_dims.y)});
    }

    Gla::Renderer::Clear();
    glClearColor(0.8, 0.8, 0.8, 1.0);

    group_bckg.Bind();
    UpdateVboBckg(vbo_bckg, shader_bckg, proj_mat,
                  app_state.project.GetCanvasDims());
    Gla::Renderer::DrawArrays(Gla::DrawMode::kTriangleStrip, 4);

    if (vbo_update_future.valid()) { vbo_update_future.wait(); }

    Gla::FrameBuffer::BindToDefaultFB();
}

void RenderLayerTextures(AppState& app_state, Gla::VertexArray& vao_canvas,
                         Gla::Shader& shader_canvas, Gla::PixelBuffer& pbo,
                         Gla::PboMappedBuffSpan& pbo_buff)
{
    vao_canvas.Bind();
    shader_canvas.Bind();
    auto proj_mat =
        GetProjMat(app_state.camera, app_state.project.GetCanvasDims());
    shader_canvas.SetUniformMat4f("u_ViewProjection", proj_mat);
    shader_canvas.SetUniform1i("u_Texture", 0);

    // NORMAL LAYER RENDERING
    for (const auto& layer : app_state.layers.GetLayers())
    {
        if (!layer.IsVisible()) { continue; }

        shader_canvas.SetUniform1i("u_Opacity", layer.GetOpacity());

        if (&layer != &app_state.layers.GetCurrentLayer())
        {
            layer.GetTexture().Bind();
            glDrawArrays(GL_TRIANGLES, 0, 6);
            layer.GetTexture().Unbind();
            continue;
        }

        auto& lay_tex = app_state.layers.GetCurrentLayerTexture();
        pbo.Bind();
        lay_tex.Bind();

        Gla::PixelBuffer::Unmap();

        lay_tex.UpdateWholeTexture(app_state.project.GetCanvasDims(), nullptr);

        glDrawArrays(GL_TRIANGLES, 0, 6);

        pbo_buff = pbo.Map();

        Gla::PixelBuffer::Unbind();
        lay_tex.Unbind();
    }

    Gla::Shader::Unbind();
    Gla::VertexArray::Unbind();
}

void RenderPreviewLayer(AppState& app_state, Gla::VertexArray& vao_canvas,
                        Gla::Shader& shader_canvas,
                        Gla::Texture2D& preview_layer_tex,
                        Gla::PixelBuffer& pbo_prev_layer,
                        Gla::PboMappedBuffSpan& preview_layer_pbo_buff)
{
    vao_canvas.Bind();
    shader_canvas.Bind();
    preview_layer_tex.Bind();

    if (app_state.preview_layer->IsPreviewLayerChanged())
    {
        pbo_prev_layer.Bind();

        Gla::PixelBuffer::Unmap();
        preview_layer_tex.UpdateWholeTexture(app_state.project.GetCanvasDims(),
                                             nullptr);
        preview_layer_pbo_buff = pbo_prev_layer.Map();

        Gla::PixelBuffer::Unbind();
    }

    auto canvas_coord_behind_cursor =
        app_state.layers.CanvasCoordsFromCursorPos();
    if (canvas_coord_behind_cursor.has_value() &&
        app_state.ui_state.ShouldDoTool())
    {
        glm::mat4 trans_mat = GetTransMat(canvas_coord_behind_cursor.value(),
                                          app_state.layers.GetCanvasDims());
        auto proj_mat =
            GetProjMat(app_state.camera, app_state.project.GetCanvasDims());
        glm::mat4 result = proj_mat;

        if (app_state.preview_layer->ShouldApplyCursorBasedTranslation())
        {
            result *= trans_mat;
        }

        shader_canvas.SetUniformMat4f("u_ViewProjection", result);
        shader_canvas.SetUniform1i("u_Texture", 0);

        glDrawArrays(GL_TRIANGLES, 0, 6);
    }

    preview_layer_tex.Unbind();
    Gla::Shader::Unbind();
    Gla::VertexArray::Unbind();
}

void RenderPreviewLayerForSelection(
    AppState& app_state, Gla::VertexArray& vao_canvas,
    Gla::Shader& shader_canvas, Gla::Texture2D& preview_layer_for_selection_tex,
    Gla::PixelBuffer& pbo_prev_layer_for_selection,
    Gla::PboMappedBuffSpan& preview_layer_for_selection_pbo_buff)
{
    vao_canvas.Bind();
    shader_canvas.Bind();
    preview_layer_for_selection_tex.Bind();

    if (app_state.preview_layer_for_selection->IsPreviewLayerChanged())
    {
        pbo_prev_layer_for_selection.Bind();

        Gla::PixelBuffer::Unmap();
        preview_layer_for_selection_tex.UpdateWholeTexture(
            app_state.project.GetCanvasDims(), nullptr);
        preview_layer_for_selection_pbo_buff =
            pbo_prev_layer_for_selection.Map();

        Gla::PixelBuffer::Unbind();
    }

    auto proj_mat =
        GetProjMat(app_state.camera, app_state.project.GetCanvasDims());
    glm::mat4 result = proj_mat;

    shader_canvas.SetUniformMat4f("u_ViewProjection", result);
    shader_canvas.SetUniform1i("u_Texture", 0);

    glDrawArrays(GL_TRIANGLES, 0, 6);

    preview_layer_for_selection_tex.Unbind();
    Gla::Shader::Unbind();
    Gla::VertexArray::Unbind();
}

void MainLoop(GLFWwindow* window)
{
    Gla::PboMappedBuffSpan pbo_buff;
    Gla::PboMappedBuffSpan preview_layer_pbo_buff;
    Gla::PboMappedBuffSpan preview_layer_for_selection_pbo_buff;
    AppState app_state{window, pbo_buff};

    Pikzel::Events::PushToScrollCallback(
        [&app_state](double x_offset, double y_offset)
        {
            if (app_state.ui_state.ShouldDoTool())
            {
                app_state.camera.ScrollCallback(x_offset, y_offset);
            }
        });
    Pikzel::Events::PushToCursorPosCallback(
        [&app_state](double x_offset, double y_offset)
        {
            if (app_state.ui_state.ShouldDoTool())
            {
                app_state.camera.CursorPosCallback(x_offset, y_offset);
            }
        });

    Gla::FrameBuffer imgui_window_fb(
        {.width = kWindowWidth, .height = kWindowHeight});

    Gla::Texture2D brush_tool_texture("assets/brush_tool.png",
                                      Gla::GLMinMagFilter::kNearest, true);
    Gla::Texture2D eraser_tool_texture("assets/eraser_tool.png",
                                       Gla::GLMinMagFilter::kNearest);
    Gla::Texture2D color_picker_tool_texture("assets/color_picker_tool.png",
                                             Gla::GLMinMagFilter::kNearest);
    Gla::Texture2D bucket_tool_texture("assets/bucket_tool.png",
                                       Gla::GLMinMagFilter::kNearest);
    Gla::Texture2D square_tool_texture("assets/square_tool.png");
    Gla::Texture2D selection_tool_texture("assets/selection_tool.png",
                                          Gla::GLMinMagFilter::kNearest);

    std::array<unsigned int,
               static_cast<std::size_t>(Pikzel::ToolType::kToolCount)>
        tool_texture_ids = {
            brush_tool_texture.GetID(),        eraser_tool_texture.GetID(),
            color_picker_tool_texture.GetID(), bucket_tool_texture.GetID(),
            square_tool_texture.GetID(),       selection_tool_texture.GetID(),
        };

    Gla::Texture2D eye_opened_texture("assets/eye_opened.png");
    Gla::Texture2D eye_closed_texture("assets/eye_closed.png");
    Gla::Texture2D lock_locked_texture("assets/lock_locked.png");
    Gla::Texture2D lock_unlocked_texture("assets/lock_unlocked.png");

    std::array<unsigned int, 4> layer_texture_ids = {
        eye_opened_texture.GetID(), eye_closed_texture.GetID(),
        lock_locked_texture.GetID(), lock_unlocked_texture.GetID()};

    app_state.ui_state.SetupToolTextures(tool_texture_ids);
    app_state.ui_state.SetupLayerToolTextures(layer_texture_ids);

    Gla::Texture2D texture_canvas({32, 32}, {0.5F, 0.5F, 0.5F, 0.5F},
                                  Gla::kNearest);
    Gla::Texture2D preview_layer_tex({32, 32}, {0.5F, 0.5F, 0.5F, 0.5F},
                                     Gla::kNearest);
    Gla::Texture2D preview_layer_for_selection_tex(
        {32, 32}, {0.5F, 0.5F, 0.5F, 0.5F}, Gla::kNearest);
    Gla::VertexArray vao_canvas;
    Gla::VertexBufferLayout layout_canvas;
    layout_canvas.Push<float>(2);
    layout_canvas.Push<float>(2);
    Gla::VertexBuffer vbo_canvas(nullptr, 24 * sizeof(float));
    vao_canvas.AddBuffer(vbo_canvas, layout_canvas);
    Gla::Shader shader_canvas("shader/layer_tex_shader.vert",
                              "shader/layer_tex_shader.frag");

    std::array<float, 8> bckg_vertices = {
        -1.0F, 1.0F,

        1.0F,  1.0F,

        -1.0F, -1.0F,

        1.0F,  -1.0F,
    };

    Gla::VertexBufferLayout bckg_layout;
    bckg_layout.Push<float>(2);

    Gla::VertexArray vao_bckg;
    Gla::VertexBuffer vbo_bckg(bckg_vertices.data(),
                               bckg_vertices.size() * sizeof bckg_vertices[0],
                               Gla::kDynamicDraw);
    vao_bckg.AddBuffer(vbo_bckg, bckg_layout);
    Gla::Shader shader_bckg("shader/background_shader.vert",
                            "shader/background_shader.frag");
    shader_bckg.Bind();
    Gla::Group group_bckg(vao_bckg, shader_bckg);

    Gla::PixelBuffer pbo(glm::ivec2{32}, {.r = 0, .g = 0, .b = 0, .a = 0});
    Gla::PixelBuffer pbo_prev_layer(glm::ivec2{32},
                                    {.r = 0, .g = 0, .b = 0, .a = 0});
    Gla::PixelBuffer pbo_prev_layer_for_selection(
        glm::ivec2{32}, {.r = 0, .g = 0, .b = 0, .a = 0});
    Gla::PixelBuffer::Unbind();

    Gla::Timer out_of_loop_timer;

    while (glfwWindowShouldClose(window) == 0)
    {
#ifndef NDEBUG
        Gla::Timer timer;
#endif

        HandleInputAndUI(app_state, imgui_window_fb, shader_bckg, vbo_canvas,
                         pbo, pbo_buff, pbo_prev_layer, preview_layer_pbo_buff,
                         pbo_prev_layer_for_selection,
                         preview_layer_for_selection_pbo_buff);

        if (app_state.project.IsOpened() &&
            app_state.ui_state.IsDrawWindowRendered())
        {
            assert(app_state.preview_layer.has_value());

            Update(app_state);

            Render(app_state, imgui_window_fb, group_bckg, shader_bckg,
                   vbo_bckg);

            imgui_window_fb.Bind();
            RenderLayerTextures(app_state, vao_canvas, shader_canvas, pbo,
                                pbo_buff);
            RenderPreviewLayerForSelection(
                app_state, vao_canvas, shader_canvas,
                preview_layer_for_selection_tex, pbo_prev_layer_for_selection,
                preview_layer_for_selection_pbo_buff);
            RenderPreviewLayer(app_state, vao_canvas, shader_canvas,
                               preview_layer_tex, pbo_prev_layer,
                               preview_layer_pbo_buff);
            Gla::FrameBuffer::BindToDefaultFB();
        }

        glfwSwapBuffers(window);

#ifndef NDEBUG
        float fps = 1.0F / timer.GetTime();
        if (out_of_loop_timer.GetTime() > 0.2)
        {
            std::string win_title = "Pikzel - FPS: " + std::to_string(fps);
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
        std::println("Failed to create window");
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
    std::println("C++ standard: {}", __cplusplus);
#endif

    if (glewInit() != GLEW_OK)
    {
        std::println(
            "Glew init error: {}",
            std::bit_cast<const char*>(glewGetErrorString(glewInit())));
        return 1;
    }

#ifndef NDEBUG
    std::println("OpenGL version: {}",
                 std::bit_cast<const char*>(glGetString(GL_VERSION)));
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
