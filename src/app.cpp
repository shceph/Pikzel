#include "app.hpp"

#include "camera.hpp"
#include "events.hpp"
#include "tool.hpp"

#include "gla/frame_buffer.hpp"
#include "gla/group.hpp"
#include "gla/pixel_buffer.hpp"
#include "gla/renderer.hpp"
#include "gla/shader.hpp"
#include "gla/texture.hpp"
#include "gla/timer.hpp"
#include "gla/vertex_array.hpp"
#include "gla/vertex_buffer.hpp"
#include "gla/vertex_buffer_layout.hpp"

#include <glad/gl.h>

#include <GLFW/glfw3.h>

#include <imgui.h>

#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <glm/glm.hpp>

#include <array>
#include <cassert>
#include <cstddef>
#include <cstdlib>
#include <iostream>
#include <optional>
#include <print>
#include <span>
#include <string>

namespace {
auto ImVec2Equal(ImVec2 vec_a, ImVec2 vec_b) -> bool {
    constexpr float kAllowedDiff = 0.01F;

    return (std::abs(vec_a.x - vec_b.x) <= kAllowedDiff &&
            std::abs(vec_a.y - vec_b.y) <= kAllowedDiff);
}
} // namespace

namespace Pikzel {
App::App(GLFWwindow* window) : mWindow{window}, mAppState{window} {}

void App::MainLoop() {
    constexpr Vec2 kCanvasDefaultDims{32};
    InitMouseCallbacks();

    Gla::PixelBuffer pbo_canvas{kCanvasDefaultDims, Gla::Color{}};
    Gla::PixelBuffer pbo_prev_layer{kCanvasDefaultDims, Gla::Color{}};
    Gla::PixelBuffer pbo_prev_layer_for_selection{kCanvasDefaultDims,
                                                  Gla::Color{}};
    Gla::PixelBuffer::Unbind();

    Gla::FrameBuffer draw_window_fbo{
        {.width = kCanvasDefaultDims.x, .height = kCanvasDefaultDims.y}};

    std::array<unsigned int, static_cast<std::size_t>(ToolType::kToolCount)>
        tool_window_tex_ids{};
    std::array<unsigned int, 4> layer_window_tex_ids{};

    LoadGuiTextures(tool_window_tex_ids, layer_window_tex_ids);

    Gla::VertexArray vao_canvas;
    Gla::VertexBufferLayout layout_canvas;
    layout_canvas.Push<float>(2);
    layout_canvas.Push<float>(2);
    Gla::VertexBuffer vbo_canvas{nullptr, kVboCanvasSize * sizeof(float)};
    vao_canvas.AddBuffer(vbo_canvas, layout_canvas);
    Gla::Shader shader_canvas{"shader/layer_tex_shader.vert",
                              "shader/layer_tex_shader.frag"};
    Gla::Group render_group_canvas{vao_canvas, vbo_canvas, shader_canvas};

    constexpr std::size_t kCanvasBckgVertexElemensCount = 8;
    std::array<float, kCanvasBckgVertexElemensCount> bckg_vertices = {
        -1.0F, 1.0F,

        1.0F,  1.0F,

        -1.0F, -1.0F,

        1.0F,  -1.0F,
    };

    Gla::VertexBufferLayout layout_bckg;
    layout_bckg.Push<float>(2);
    Gla::VertexArray vao_bckg;
    Gla::VertexBuffer vbo_bckg{bckg_vertices.data(),
                               bckg_vertices.size() * sizeof bckg_vertices[0],
                               Gla::kDynamicDraw};
    vao_bckg.AddBuffer(vbo_bckg, layout_bckg);
    Gla::Shader shader_bckg("shader/background_shader.vert",
                            "shader/background_shader.frag");
    shader_bckg.Bind();
    Gla::Group render_group_canvas_bckg{vao_bckg, vbo_bckg, shader_bckg};

    Gla::Timer out_of_loop_timer;

    while (glfwWindowShouldClose(mWindow) == 0) {
#ifndef NDEBUG
        Gla::Timer timer;
#endif

        bool const should_create_new_project =
            HandleInputAndUI(draw_window_fbo);

        if (should_create_new_project) {
            CreateNewProject(shader_bckg, vbo_canvas, pbo_canvas,
                             pbo_prev_layer, pbo_prev_layer_for_selection);
        }

        if (mAppState.project.IsOpened() &&
            mAppState.ui_state.IsDrawWindowRendered()) {
            assert(mAppState.preview_layer.has_value());
            assert(mAppState.preview_layer_for_selection.has_value());

            Update(pbo_canvas, draw_window_fbo);
            Render(render_group_canvas, render_group_canvas_bckg,
                   draw_window_fbo, pbo_prev_layer,
                   pbo_prev_layer_for_selection);
        }

        glfwSwapBuffers(mWindow);

#ifndef NDEBUG
        constexpr double kFpsUpdateInterval = 0.2;
        const float fps = 1.0F / timer.GetTime();

        if (out_of_loop_timer.GetTime() > kFpsUpdateInterval) {
            const std::string win_title =
                "Pikzel - FPS: " + std::to_string(fps);
            glfwSetWindowTitle(mWindow, win_title.c_str());
            out_of_loop_timer.Reset();
        }
#endif
    }
}

void App::InitMouseCallbacks() {
    Events::PushToScrollCallback([this](double x_offset, double y_offset) {
        if (mAppState.ui_state.ShouldDoTool()) {
            mAppState.camera.ScrollCallback(x_offset, y_offset);
        }
    });
    Events::PushToCursorPosCallback([this](double x_offset, double y_offset) {
        if (mAppState.ui_state.ShouldDoTool()) {
            mAppState.camera.CursorPosCallback(x_offset, y_offset);
        }
    });
}

void App::LoadGuiTextures(std::span<unsigned int> tool_window_tex_ids,
                          std::span<unsigned int> layer_window_tex_ids) {
    tool_window_tex_ids[static_cast<std::size_t>(ToolType::kBrush)] =
        mBrushToolTexture.GetID();
    tool_window_tex_ids[static_cast<std::size_t>(ToolType::kEraser)] =
        mEraserToolTexture.GetID();
    tool_window_tex_ids[static_cast<std::size_t>(ToolType::kColorPicker)] =
        mColorPickerToolTexture.GetID();
    tool_window_tex_ids[static_cast<std::size_t>(ToolType::kBucket)] =
        mBucketToolTexture.GetID();
    tool_window_tex_ids[static_cast<std::size_t>(ToolType::kRectShape)] =
        mSquareToolTexture.GetID();
    tool_window_tex_ids[static_cast<std::size_t>(ToolType::kSelectionTool)] =
        mSelectionToolTexture.GetID();
    tool_window_tex_ids[static_cast<std::size_t>(ToolType::kColorSelection)] =
        mColorSelectionToolTexture.GetID();
    tool_window_tex_ids[static_cast<std::size_t>(ToolType::kMoveSelection)] =
        mMoveToolTexture.GetID();

    layer_window_tex_ids[static_cast<std::size_t>(UI::kEyeOpenedTexture)] =
        mEyeOpenedTexture.GetID();
    layer_window_tex_ids[static_cast<std::size_t>(UI::kEyeClosedTexture)] =
        mEyeClosedTexture.GetID();
    layer_window_tex_ids[static_cast<std::size_t>(UI::kLockLockedTexture)] =
        mLockLockedTexture.GetID();
    layer_window_tex_ids[static_cast<std::size_t>(UI::kLockUnlockedTexture)] =
        mLockUnlockedTexture.GetID();

    mAppState.ui_state.SetupToolTextures(tool_window_tex_ids);
    mAppState.ui_state.SetupLayerToolTextures(layer_window_tex_ids);
}

auto App::HandleInputAndUI(Gla::FrameBuffer& draw_window_fbo)
    -> App::ShouldCreateNewProject {
    Events::Update();

    UI::NewFrame();

    if (mAppState.project.IsOpened()) {
        assert(mAppState.preview_layer_for_selection.has_value());
        mAppState.ui_state.RenderUI(mAppState.layers, mAppState.camera,
                                    mAppState.layers.GetSelection(),
                                    *mAppState.preview_layer_for_selection);
        mAppState.ui_state.RenderDrawWindow(draw_window_fbo.GetTextureID(),
                                            "Draw");
        UI::RenderAndEndFrame(mWindow);
        return false;
    }

    mAppState.ui_state.RenderNoProjectWindow();
    UI::RenderAndEndFrame(mWindow);

    // Project may have been opened in RenderNoProjectWindow
    return mAppState.project.IsOpened();
}

void App::CreateNewProject(Gla::Shader& shader_bckg,
                           Gla::VertexBuffer& vbo_canvas,
                           Gla::PixelBuffer& pbo_canvas,
                           Gla::PixelBuffer& pbo_prev_layer,
                           Gla::PixelBuffer& pbo_prev_layer_for_selection) {
    mAppState.preview_layer.emplace(mAppState.tool, mAppState.camera,
                                    mAppState.layers.GetCanvasDims());
    mAppState.preview_layer_for_selection.emplace(
        mAppState.tool, mAppState.camera, mAppState.layers.GetCanvasDims());

    const float height_over_width =
        static_cast<float>(mAppState.project.CanvasHeight()) /
        static_cast<float>(mAppState.project.CanvasWidth());
    shader_bckg.Bind();
    shader_bckg.SetUniform1f("u_CanvasHeightOverWidth", height_over_width);

    auto can_width = static_cast<float>(mAppState.project.CanvasWidth());
    auto can_height = static_cast<float>(mAppState.project.CanvasHeight());

    std::array<float, kVboCanvasSize> vbo_canvas_vertices = {
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

    pbo_canvas.BindAndResize(mAppState.project.GetCanvasDims(), Gla::Color{});
    mAppState.layers.UpdateMappedPBOMemorySpanForAllLayers(pbo_canvas.Map());
    Gla::PixelBuffer::Unbind();

    pbo_prev_layer.BindAndResize(mAppState.project.GetCanvasDims(),
                                 Gla::Color{});
    assert(mAppState.preview_layer.has_value() &&
           "preview_layer is std::nullopt");
    mAppState.preview_layer->UpdatePBOMappedBufferSpan(pbo_prev_layer.Map());
    Gla::PixelBuffer::Unbind();

    pbo_prev_layer_for_selection.BindAndResize(
        mAppState.project.GetCanvasDims(), Gla::Color{});
    assert(mAppState.preview_layer_for_selection.has_value() &&
           "preview_layer_for_selection is std::nullopt");
    mAppState.preview_layer_for_selection->UpdatePBOMappedBufferSpan(
        pbo_prev_layer_for_selection.Map());
    Gla::PixelBuffer::Unbind();
}

void App::Update(Gla::PixelBuffer& pbo_canvas,
                 Gla::FrameBuffer& draw_window_fbo) {
    if (mAppState.layers.HaveChosenDifferentLayerThisFrame()) {
        mAppState.layers.WriteCurrentLayerTextureDataToPbo();
    }

    if (!mAppState.preview_layer.has_value()) {
        std::println(std::cerr, "preview_layer is nullopt");
        std::abort();
    }

    if (!mAppState.preview_layer_for_selection.has_value()) {
        std::println(std::cerr, "preview_layer_for_selection is nullopt");
        std::abort();
    }

    auto& preview_layer = *mAppState.preview_layer;
    auto& preview_layer_for_selection = *mAppState.preview_layer_for_selection;

    mAppState.preview_layer->Update();

    auto win_data = mAppState.ui_state.CreateCanvasWindowData();

    mAppState.layers.UpdateAndDraw(
        mAppState.ui_state.ShouldDoTool(), mAppState.tool, mAppState.camera,
        preview_layer, preview_layer_for_selection, pbo_canvas, win_data,
        mAppState.ui_state.GetColorSelectionThreshold());
    mAppState.ui_state.Update();

    UpdateDrawWindowFrameBuffer(draw_window_fbo);
}

void App::UpdateDrawWindowFrameBuffer(Gla::FrameBuffer& draw_window_fbo) const {
    static const ImVec2 kDrawWindowDims;

    draw_window_fbo.Bind();

    if (!ImVec2Equal(kDrawWindowDims,
                     mAppState.ui_state.GetDrawWinDimensions())) {
        const ImVec2 draw_window_dims =
            mAppState.ui_state.GetDrawWinDimensions();
        draw_window_fbo.Rescale(
            {.width = static_cast<int>(draw_window_dims.x),
             .height = static_cast<int>(draw_window_dims.y)});
    }

    Gla::FrameBuffer::BindToDefaultFB();
}

void App::Render(Gla::Group& render_group_canvas,
                 Gla::Group& render_group_canvas_bckg,
                 const Gla::FrameBuffer& draw_window_fbo,
                 Gla::PixelBuffer& pbo_prev_layer,
                 Gla::PixelBuffer& pbo_prev_layer_for_selection) {
    draw_window_fbo.Bind();
    RenderBackground(render_group_canvas_bckg);
    RenderLayerTextures(render_group_canvas);
    RenderPreviewLayerForSelection(render_group_canvas,
                                   pbo_prev_layer_for_selection);
    RenderPreviewLayer(render_group_canvas, pbo_prev_layer);
    Gla::FrameBuffer::BindToDefaultFB();
}

void App::RenderBackground(Gla::Group& render_group_canvas_bckg) const {
    auto proj_mat =
        GetProjMat(mAppState.camera, mAppState.project.GetCanvasDims());

    Gla::Renderer::Clear();
    constexpr glm::vec4 kClearColor{0.8F, 0.8F, 0.8F, 1.0F};
    glClearColor(kClearColor.r, kClearColor.g, kClearColor.b, kClearColor.a);

    render_group_canvas_bckg.Bind();
    render_group_canvas_bckg.GetVbo().Bind();
    UpdateVboBckg(render_group_canvas_bckg, proj_mat,
                  mAppState.project.GetCanvasDims());
    Gla::VertexBuffer::Unbind();
    Gla::Renderer::DrawArrays(Gla::DrawMode::kTriangleStrip, 4);
    render_group_canvas_bckg.Unbind();
}

void App::RenderLayerTextures(Gla::Group& render_group_canvas) const {
    const auto& vao_canvas = render_group_canvas.GetVao();
    auto& shader_canvas = render_group_canvas.GetShader();

    vao_canvas.Bind();
    shader_canvas.Bind();
    auto proj_mat =
        GetProjMat(mAppState.camera, mAppState.project.GetCanvasDims());
    shader_canvas.SetUniformMat4f("u_ViewProjection", proj_mat);
    shader_canvas.SetUniform1i("u_Texture", 0);

    for (const auto& layer : mAppState.layers.GetLayers()) {
        if (!layer.IsVisible()) {
            continue;
        }

        shader_canvas.SetUniform1i("u_Opacity", layer.GetOpacity());

        layer.GetTexture().Bind();
        constexpr GLsizei kVertexCount = 6;
        Gla::Renderer::DrawArrays(Gla::DrawMode::kTriangles, kVertexCount);
        layer.GetTexture().Unbind();
    }

    Gla::Shader::Unbind();
    Gla::VertexArray::Unbind();
}

void App::RenderPreviewLayer(Gla::Group& render_group_canvas,
                             Gla::PixelBuffer& pbo_prev_layer) {
    const auto& vao_canvas = render_group_canvas.GetVao();
    auto& shader_canvas = render_group_canvas.GetShader();
    auto win_data = mAppState.ui_state.CreateCanvasWindowData();

    if (!mAppState.preview_layer.has_value()) {
        std::println(std::cerr, "preview_layer is nullopt");
        std::abort();
    }

    auto& preview_layer = *mAppState.preview_layer;

    vao_canvas.Bind();
    shader_canvas.Bind();
    Gla::Texture2D& preview_layer_tex = preview_layer.GetLayerTexture();
    preview_layer_tex.Bind();

    if (preview_layer.IsPreviewLayerChanged()) {
        pbo_prev_layer.Bind();

        pbo_prev_layer.Unmap();
        preview_layer_tex.UpdateWholeTexture(mAppState.project.GetCanvasDims(),
                                             nullptr);
        preview_layer.UpdatePBOMappedBufferSpan(pbo_prev_layer.Map());

        Gla::PixelBuffer::Unbind();
    }

    std::optional<Vec2> canvas_coord_behind_cursor =
        mAppState.layers.CanvasCoordsFromCursorPos(win_data);

    if (canvas_coord_behind_cursor.has_value() &&
        mAppState.ui_state.ShouldDoTool()) {
        auto proj_mat =
            GetProjMat(mAppState.camera, mAppState.project.GetCanvasDims());
        glm::mat4 result = proj_mat;

        if (preview_layer.ShouldApplyCursorBasedTranslation()) {
            const glm::mat4 trans_mat =
                GetTransMatForPreviewLayer(canvas_coord_behind_cursor.value(),
                                           mAppState.layers.GetCanvasDims());
            result *= trans_mat;
        }

        shader_canvas.SetUniformMat4f("u_ViewProjection", result);
        shader_canvas.SetUniform1i("u_Texture", 0);

        constexpr GLsizei kVertexCount = 6;
        Gla::Renderer::DrawArrays(Gla::DrawMode::kTriangles, kVertexCount);
    }

    preview_layer_tex.Unbind();
    Gla::Shader::Unbind();
    Gla::VertexArray::Unbind();
}

void App::RenderPreviewLayerForSelection(
    Gla::Group& render_group_canvas,
    Gla::PixelBuffer& pbo_prev_layer_for_selection) {

    const auto& vao_canvas = render_group_canvas.GetVao();
    auto& shader_canvas = render_group_canvas.GetShader();

    if (!mAppState.preview_layer_for_selection.has_value()) {
        std::println(std::cerr, "preview_layer_for_selection is nullopt");
        std::abort();
    }

    auto& preview_layer_for_selection = *mAppState.preview_layer_for_selection;

    vao_canvas.Bind();
    shader_canvas.Bind();
    Gla::Texture2D& preview_layer_for_selection_tex =
        preview_layer_for_selection.GetLayerTexture();
    preview_layer_for_selection_tex.Bind();

    if (preview_layer_for_selection.IsPreviewLayerChanged()) {
        pbo_prev_layer_for_selection.Bind();

        pbo_prev_layer_for_selection.Unmap();
        preview_layer_for_selection_tex.UpdateWholeTexture(
            mAppState.project.GetCanvasDims(), nullptr);
        preview_layer_for_selection.UpdatePBOMappedBufferSpan(
            pbo_prev_layer_for_selection.Map());

        Gla::PixelBuffer::Unbind();
    }

    auto proj_mat =
        GetProjMat(mAppState.camera, mAppState.project.GetCanvasDims());
    const glm::mat4 result = proj_mat;

    shader_canvas.SetUniformMat4f("u_ViewProjection", result);
    shader_canvas.SetUniform1i("u_Texture", 0);

    constexpr GLsizei kVertexCount = 6;
    Gla::Renderer::DrawArrays(Gla::DrawMode::kTriangles, kVertexCount);

    preview_layer_for_selection_tex.Unbind();
    Gla::Shader::Unbind();
    Gla::VertexArray::Unbind();
}

void App::UpdateVboBckg(Gla::Group& render_group_canvas_bckg,
                        glm::mat4 canvas_mat, Pikzel::Vec2 canvas_dims) {
    auto& vbo_bckg = render_group_canvas_bckg.GetVbo();
    auto& shader_bckg = render_group_canvas_bckg.GetShader();

    const glm::vec2 top_left = canvas_mat * glm::vec4{0, 0, 0, 1};
    const glm::vec2 bottom_right = canvas_mat * glm::vec4{canvas_dims, 0, 1};

    constexpr std::size_t kCanvasBckgVertexElemensCount = 8;

    std::array<float, kCanvasBckgVertexElemensCount> bckg_vertices = {
        top_left.x,     top_left.y,

        bottom_right.x, top_left.y,

        top_left.x,     bottom_right.y,

        bottom_right.x, bottom_right.y,
    };
    vbo_bckg.UpdateData(bckg_vertices.data(),
                        kCanvasBckgVertexElemensCount * sizeof(float));

    const glm::vec2 top_left_in_uv = (top_left + glm::vec2{1}) / glm::vec2{2};
    shader_bckg.SetUniform2f("u_TopLeftInUV", top_left_in_uv.x,
                             top_left_in_uv.y);
}

auto App::GetProjMat(const Camera& camera, Vec2 canvas_dims) -> glm::mat4 {
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

auto App::GetTransMatForPreviewLayer(Vec2 canvas_coord_behind_cursor,
                                     Vec2 canvas_dims) -> glm::mat4 {
    glm::mat4 translation_mat(1.0);
    auto move_distance = canvas_coord_behind_cursor - (canvas_dims / 2);

    translation_mat = glm::translate(
        glm::mat4(1.0), glm::vec3(move_distance.x, move_distance.y, 1.0));

    return translation_mat;
}
} // namespace Pikzel
