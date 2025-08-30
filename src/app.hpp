#pragma once

#include "layer_control.hpp"
#include "project.hpp"
#include "tool.hpp"
#include "camera.hpp"
#include "gui.hpp"

#include "gla/frame_buffer.hpp"
#include "gla/vertex_buffer.hpp"
#include "gla/shader.hpp"
#include "gla/group.hpp"

#include <GLFW/glfw3.h>

#include <optional>
#include <span>

namespace Pikzel {
struct AppState {
    explicit AppState(GLFWwindow* window)
        : camera{}, project{layers, tool, camera},
          ui_state{project, tool, window}, preview_layer{std::nullopt},
          preview_layer_for_selection{std::nullopt} {}

    Pikzel::Tool tool;
    Pikzel::Camera camera;
    Pikzel::LayerControl layers;
    Pikzel::Project project;
    Pikzel::UI ui_state;
    std::optional<Pikzel::PreviewLayer> preview_layer;
    std::optional<Pikzel::PreviewLayer> preview_layer_for_selection;
};

class App {
  public:
    explicit App(GLFWwindow* window);
    void MainLoop();

  private:
    using ShouldCreateNewProject = bool;
    void InitMouseCallbacks();
    void LoadGuiTextures(std::span<unsigned int> tool_window_tex_ids,
                         std::span<unsigned int> layer_window_tex_ids);
    auto HandleInputAndUI(Gla::FrameBuffer& draw_window_fbo)
        -> ShouldCreateNewProject;
    void CreateNewProject(Gla::Shader& shader_bckg,
                          Gla::VertexBuffer& vbo_canvas,
                          Gla::PixelBuffer& pbo_canvas,
                          Gla::PixelBuffer& pbo_prev_layer,
                          Gla::PixelBuffer& pbo_prev_layer_for_selection);
    void Update(Gla::PixelBuffer& pbo_canvas,
                Gla::FrameBuffer& draw_window_fbo);
    void UpdateDrawWindowFrameBuffer(Gla::FrameBuffer& draw_window_fbo) const;
    void Render(Gla::Group& render_group_canvas,
                Gla::Group& render_group_canvas_bckg,
                const Gla::FrameBuffer& draw_window_fbo,
                Gla::PixelBuffer& pbo_prev_layer,
                Gla::PixelBuffer& pbo_prev_layer_for_selection);
    void RenderBackground(Gla::Group& render_group_canvas_bckg) const;
    void RenderLayerTextures(Gla::Group& render_group_canvas) const;
    void RenderPreviewLayer(Gla::Group& render_group_canvas,
                            Gla::PixelBuffer& pbo_prev_layer);
    void RenderPreviewLayerForSelection(
        Gla::Group& render_group_canvas,
        Gla::PixelBuffer& pbo_prev_layer_for_selection);
    static void UpdateVboBckg(Gla::Group& render_group_canvas_bckg,
                              glm::mat4 canvas_mat, Pikzel::Vec2 canvas_dims);
    static auto GetProjMat(const Pikzel::Camera& camera,
                           Pikzel::Vec2 canvas_dims) -> glm::mat4;
    // Moves previews of certain tools. For example, we want brush preview to be
    // behind the cursor, so we would know where we are drawing. Instead of
    // updating the preview layer whenever we move the cursor, which is really
    // inefficient, we just translate the preview layer in the shader
    static auto
    GetTransMatForPreviewLayer(Pikzel::Vec2 canvas_coord_behind_cursor,
                               Pikzel::Vec2 canvas_dims) -> glm::mat4;

    // 6 vertices with 4 elements; 2 for pos and 2 for uv
    static constexpr std::size_t kVboCanvasSize = 6UZ * 4UZ;

    GLFWwindow* mWindow;
    AppState mAppState;

    Gla::Texture2D mBrushToolTexture{"assets/brush_tool.png",
                                     Gla::GLMinMagFilter::kNearest, true};
    Gla::Texture2D mEraserToolTexture{"assets/eraser_tool.png",
                                      Gla::GLMinMagFilter::kNearest};
    Gla::Texture2D mColorPickerToolTexture{"assets/color_picker_tool.png",
                                           Gla::GLMinMagFilter::kNearest};
    Gla::Texture2D mBucketToolTexture{"assets/bucket_tool.png",
                                      Gla::GLMinMagFilter::kNearest};
    Gla::Texture2D mSquareToolTexture{"assets/square_tool.png"};
    Gla::Texture2D mSelectionToolTexture{"assets/selection_tool.png",
                                         Gla::GLMinMagFilter::kNearest};
    Gla::Texture2D mColorSelectionToolTexture{"assets/color_selection.png",
                                              Gla::GLMinMagFilter::kNearest};
    Gla::Texture2D mOveToolTexture{"assets/move_tool.png",
                                   Gla::GLMinMagFilter::kNearest};

    Gla::Texture2D mEyeOpenedTexture{"assets/eye_opened.png"};
    Gla::Texture2D mEyeClosedTexture{"assets/eye_closed.png"};
    Gla::Texture2D mLockLockedTexture{"assets/lock_locked.png"};
    Gla::Texture2D mLockUnlockedTexture{"assets/lock_unlocked.png"};
};
} // namespace Pikzel
