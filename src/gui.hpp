#pragma once

#include "layer_control.hpp"
#include "preview_layer.hpp"
#include "project.hpp"
#include "tool.hpp"
#include "camera.hpp"
#include "selection.hpp"
#include "tree.hpp"

#include <GLFW/glfw3.h>
#include <imgui.h>

#include <cstddef>
#include <array>
#include <span>
#include <functional>

namespace Pikzel {
class UI {
  public:
    UI(Project& project, Tool& tool, GLFWwindow* _window);
    void RenderUI(LayerControl& layers, Camera& camera, Selection& selection,
                  PreviewLayer& preview_layer_for_selection);
    void RenderNoProjectWindow();
    void RenderDrawWindow(unsigned int framebuffer_texture_id,
                          const char* window_name);
    void Update();
    void SetupToolTextures(std::span<unsigned int> tex_ids);
    void SetupLayerToolTextures(std::span<unsigned int> layer_tex_ids);
    auto CreateCanvasWindowData() -> Layer::CanvasWindowData;
    static void NewFrame();
    static void RenderAndEndFrame(GLFWwindow* window);

    void SetShouldDoToolToTrue() { mShouldDoTool = true; }
    void TriggerSaveErrorPopup() { mRenderSaveErrorPopup = true; }

    [[nodiscard]] auto ShouldDoTool() const -> bool;

    [[nodiscard]] auto IsDrawWindowRendered() const -> bool {
        return mDrawWindowRendered;
    }

    [[nodiscard]] auto GetDrawWinDimensions() const -> ImVec2 {
        return mDrawWinDimensions;
    }

    [[nodiscard]] auto GetColorSelectionThreshold() const -> int {
        return mSelectionByColorThreshold;
    }

    [[nodiscard]] auto GetWindowPointer() const -> GLFWwindow* {
        return mWindow;
    }

    [[nodiscard]] auto GetCanvasUpperleftCoords() const -> ImVec2 {
        return mCanvasUpperLeft;
    }

    [[nodiscard]] auto GetCanvasBottomRightCoords() const -> ImVec2 {
        return mCanvasBottomRight;
    }

  private:
    struct RenderNodesChildrenFuncData {
        int node_count{0};
        Tree<LayerControl::Capture>* clicked_node = nullptr;

        void Reset() {
            node_count = 0;
            clicked_node = nullptr;
        }
    };

    void RenderMenuBar(LayerControl& layers, Camera& camera,
                       Selection& selection,
                       PreviewLayer& preview_layer_for_selection);
    void RenderFileMenu();
    void RenderEditMenu(LayerControl& layers);
    static void RenderViewMenu(Camera& camera);
    static void RenderSelectionMenu(Selection& selection,
                                    PreviewLayer& preview_layer_for_selection);
    void RenderSaveAsImagePopup();
    void RenderSaveAsProjectPopup();
    void RenderNodesChildren(LayerControl& layers,
                             Tree<LayerControl::Capture>& node);
    void RenderUndoTreeWindow(LayerControl& layers);
    void RenderToolWindow();
    void RenderToolButton(ToolType tool_to_render, bool is_current,
                          const std::string& btn_id);
    void RenderLayerWindow(LayerControl& layers);
    void RenderLayerWinContextMenu(LayerControl& layers);
    void RenderSaveErrorPopup();
    void RenderNewProjectPopup();
    void RenderOpenProjectPopup();

    void RenderColorWindow();
    static void RenderColorPalette(ImVec4& color);

    // The outline around a control. Don't forget to call EndOutline!
    static void BeginOutline(
        ImVec4 outline_color = ImGui::GetStyleColorVec4(ImGuiCol_SliderGrab));
    // The outline around a control
    static void EndOutline();

    static auto GetCanvasUpperleftCoordsRef() -> ImVec2& {
        static ImVec2 can_upper_left;
        return can_upper_left;
    }

    static auto GetCanvasBottomRightCoordsRef() -> ImVec2& {
        static ImVec2 can_bottom_right;
        return can_bottom_right;
    }

    auto GetSelectedItemOutlineColor() -> ImVec4& {
        return mSelectedItemOutlineColor;
    }

    RenderNodesChildrenFuncData mRenderNodesChildrenFuncData;

    std::reference_wrapper<Tool> mTool;
    std::reference_wrapper<Project> mProject;
    std::array<ImTextureID, static_cast<std::size_t>(ToolType::kToolCount)>
        mToolTextures{};

    ImTextureID mEyeOpenedTextureID{0};
    ImTextureID mEyeClosedTextureID{0};
    ImTextureID mLockLockedTextureID{0};
    ImTextureID mLockUnlockedTextureID{0};

    ImVec2 mCanvasUpperLeft;
    ImVec2 mCanvasBottomRight;
    ImVec2 mDrawWinDimensions;
    ImVec4 mSelectedItemOutlineColor;

    bool mShouldDoTool{false};
    bool mRenderSaveAsImgPopup{false};
    bool mRenderSaveAsPrjPopup{false};
    bool mRenderSaveErrorPopup{false};
    bool mRenderNewProjectPopup{false};
    bool mRenderOpenProjectPopup{false};
    bool mRenderUndoTreeWindow{false};
    bool mDrawWindowRendered{false};

    int mSelectionByColorThreshold{0};
    GLFWwindow* mWindow{nullptr};

    inline static int sConstructCounter{0};
};
} // namespace Pikzel
