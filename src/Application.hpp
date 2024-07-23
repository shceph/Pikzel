#pragma once

#include <GLFW/glfw3.h>
#include <imgui.h>

#include "Project.hpp"

namespace Pikzel
{
class UI
{
  public:
    static void ImGuiInit(GLFWwindow* _window);
    static void ImGuiCleanup();
    static void NewFrame();
    static void RenderAndEndFrame();
    static void RenderUI();
    static void RenderNoProjectWindow();
    static void RenderDrawWindow(unsigned int framebuffer_texture_id,
                                 const char* window_name);
    static void Update();

    static void SetupToolTextures(unsigned int brush_tex_id,
                                  unsigned int eraser_tex_id,
                                  unsigned int color_pick_tex_id,
                                  unsigned int bucket_tex_id);
    static void SetupLayerToolTextures(unsigned int eye_opened_id,
                                       unsigned int eye_closed_id,
                                       unsigned int lock_locked_id,
                                       unsigned int lock_unlocked_id);

    static auto ShouldUpdateVertexBuffer() -> bool;
    static auto ShouldDoTool() -> bool;

    static auto IsDrawWindowRendered() -> bool { return sDrawWindowRendered; }

    inline static auto GetDrawWinUpperleftCoords() -> ImVec2&
    {
        static ImVec2 drwin_upperleft_coords;
        return drwin_upperleft_coords;
    }
    inline static auto GetDrawWinDimensions() -> ImVec2&
    {
        static ImVec2 drwin_dims;
        return drwin_dims;
    }

    inline static auto GetCanvasUpperleftCoords() -> ImVec2&
    {
        static ImVec2 can_upperleft_coords;
        return can_upperleft_coords;
    }
    inline static auto GetCanvasBottomRightCoords() -> ImVec2&
    {
        static ImVec2 can_bottomright_coords;
        return can_bottomright_coords;
    }

    inline static auto GetWindowPointer() -> GLFWwindow* { return sWindow; }

    inline static void SetVertexBuffUpdateToTrue()
    {
        sUpdateVertexBuffer = true;
    }
    inline static void SetVertexBuffUpdateToFalse()
    {
        sUpdateVertexBuffer = false;
    }
    inline static void SetShouldDoToolToTrue() { sShouldDoTool = true; }

  private:
    static void RenderMenuBar();
    static void RenderSaveAsImagePopup();
    static void RenderSaveAsProjectPopup();
    static void RenderColorWindow();
    static void RenderColorPalette(ImVec4& color);
    static void RenderToolWindow();
    static void RenderLayerWinContextMenu();
    static void RenderLayerWindow();
    static void RenderSaveErrorPopup();
    static void RenderNewProjectPopup();

    static void
    BeginOutline(ImVec4& outline_color = GetSelectedItemOutlineColor());
    static void EndOutline();

    inline static auto GetSelectedItemOutlineColor() -> ImVec4&
    {
        static ImVec4 sel_itm_outline_col;
        return sel_itm_outline_col;
    }

    inline static GLFWwindow* sWindow;

    inline static ImTextureID sBrushToolTextureID = nullptr;
    inline static ImTextureID sEraserToolTextureID = nullptr;
    inline static ImTextureID sColorPickerToolTextureID = nullptr;
    inline static ImTextureID sBucketToolTextureID = nullptr;

    inline static ImTextureID sEyeOpenedTextureID = nullptr;
    inline static ImTextureID sEyeClosedTextureID = nullptr;
    inline static ImTextureID sLockLockedTextureID = nullptr;
    inline static ImTextureID sLockUnlockedTextureID = nullptr;

    /* inline static ImVec4 sSelectedItemOutlineColor; */

    // Says whether the vertex buffer should be updated, or the canvas state, in
    // other words
    inline static bool sUpdateVertexBuffer = false;
    inline static bool sShouldDoTool = false;
    inline static bool sRenderSaveAsImgPopup = false;
    inline static bool sRenderSaveAsPrjPopup = false;
    inline static bool sRenderSaveErrorPopup = false;
    inline static bool sRenderNewProjectPopup = false;
    inline static bool sDrawWindowRendered = false;

    friend void Project::SaveAsImage(int, const std::string&);
};
}  // namespace Pikzel
