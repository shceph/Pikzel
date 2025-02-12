#pragma once

#include "layer.hpp"
#include "tool.hpp"

#include <GLFW/glfw3.h>
#include <imgui.h>

#include <array>
#include <memory>
#include <span>

namespace Pikzel
{
class UI
{
  public:
    UI(std::shared_ptr<Project> project, std::shared_ptr<Tool> tool,
       GLFWwindow* _window);
    void RenderUI(Layers& layers, Camera& camera);
    void RenderNoProjectWindow();
    void RenderDrawWindow(unsigned int framebuffer_texture_id,
                          const char* window_name);
    void Update();

    void SetupToolTextures(std::span<unsigned int> tex_ids);
    void SetupLayerToolTextures(unsigned int eye_opened_id,
                                unsigned int eye_closed_id,
                                unsigned int lock_locked_id,
                                unsigned int lock_unlocked_id);
    static void NewFrame();
    static void RenderAndEndFrame();

    [[nodiscard]] auto ShouldDoTool() const -> bool;

    [[nodiscard]] auto IsDrawWindowRendered() const -> bool
    {
        return mDrawWindowRendered;
    }

    [[nodiscard]] auto GetDrawWinDimensions() const -> ImVec2
    {
        return mDrawWinDimensions;
    }

    static auto GetCanvasUpperleftCoords() -> ImVec2
    {
        return GetCanvasUpperleftCoordsRef();
    }
    static auto GetCanvasBottomRightCoords() -> ImVec2
    {
        return GetCanvasBottomRightCoordsRef();
    }

    void TriggerSaveErrorPopup() { mRenderSaveErrorPopup = true; }

    [[nodiscard]] static auto GetWindowPointer() -> GLFWwindow*
    {
        return sWindow;
    }
    void SetShouldDoToolToTrue() { mShouldDoTool = true; }

  private:
    void RenderMenuBar(Layers& layers, Camera& camera);
    void RenderSaveAsImagePopup();
    void RenderSaveAsProjectPopup();
    void RenderToolWindow();
    void RenderLayerWindow(Layers& layers);
    void RenderLayerWinContextMenu(Layers& layers);
    void RenderSaveErrorPopup();
    void RenderNewProjectPopup();
    void RenderOpenProjectPopup();

    void RenderColorWindow();
    static void RenderColorPalette(ImVec4& color);
    static void BeginOutline(
        ImVec4 outline_color = ImGui::GetStyleColorVec4(ImGuiCol_SliderGrab));
    static void EndOutline();

    static auto GetCanvasUpperleftCoordsRef() -> ImVec2&
    {
        static ImVec2 can_upper_left;
        return can_upper_left;
    }

    static auto GetCanvasBottomRightCoordsRef() -> ImVec2&
    {
        static ImVec2 can_bottom_right;
        return can_bottom_right;
    }

    auto GetSelectedItemOutlineColor() -> ImVec4&
    {
        return mSelectedItemOutlineColor;
    }

    std::shared_ptr<Tool> mTool;
    std::shared_ptr<Project> mProject;
    std::array<ImTextureID, static_cast<std::size_t>(ToolType::kToolCount)>
        mToolTextures{};

    ImTextureID mEyeOpenedTextureID = nullptr;
    ImTextureID mEyeClosedTextureID = nullptr;
    ImTextureID mLockLockedTextureID = nullptr;
    ImTextureID mLockUnlockedTextureID = nullptr;

    ImVec2 mDrawWinDimensions;
    ImVec4 mSelectedItemOutlineColor;

    bool mShouldDoTool = false;
    bool mRenderSaveAsImgPopup = false;
    bool mRenderSaveAsPrjPopup = false;
    bool mRenderSaveErrorPopup = false;
    bool mRenderNewProjectPopup = false;
    bool mRenderOpenProjectPopup = false;
    bool mDrawWindowRendered = false;

    inline static int sConstructCounter = 0;
    inline static GLFWwindow* sWindow = nullptr;
};
} // namespace Pikzel
