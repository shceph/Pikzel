#include "Application.hpp"
#include "Camera.hpp"
#include "Layer.hpp"
#include "Tool.hpp"

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <imgui_internal.h>
#include <imgui_stdlib.h>

#include <GLFW/glfw3.h>

#include <array>
#include <bit>
#include <string>

namespace Pikzel
{
void UI::ImGuiInit(GLFWwindow* _window)
{
    sWindow = _window;

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();

    ImGuiIO& imgui_io = ImGui::GetIO();
    imgui_io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    imgui_io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;
    imgui_io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    imgui_io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;
    /* imgui_io.Fonts->AddFontFromFileTTF("assets/ProggyTiny.ttf", 10.0F); */
    /* imgui_io.Fonts->AddFontDefault(); */

    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(sWindow, true);
    ImGui_ImplOpenGL3_Init();

    GetSelectedItemOutlineColor() =
        ImGui::GetStyleColorVec4(ImGuiCol_SliderGrab);

    ImGuiStyle& style = ImGui::GetStyle();
    style.FrameRounding = 3.0F;
    style.GrabRounding = 3.0F;
    style.WindowRounding = 3.0F;
    style.ScrollbarRounding = 3.0F;
    style.TabRounding = 3.0F;
    style.ChildRounding = 3.0F;
    style.PopupRounding = 3.0F;
    /* style.FrameBorderSize = 1.0F; */
    /* style.WindowBorderSize = 1.0F; */
    /* style.ScaleAllSizes(1.5); */
}

void UI::ImGuiCleanup()
{
}

void UI::NewFrame()
{
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
}

void UI::RenderAndEndFrame()
{
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    // Update and Render additional Platform Windows
    // (Platform functions may change the current OpenGL context, so we
    // save/restore it to make it easier to paste this code elsewhere.
    // For this specific demo app we could also call
    // glfwMakeContextCurrent(window) directly)
    if ((ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_ViewportsEnable) != 0)
    {
        GLFWwindow* backup_current_context = glfwGetCurrentContext();
        ImGui::UpdatePlatformWindows();
        ImGui::RenderPlatformWindowsDefault();
        glfwMakeContextCurrent(backup_current_context);
    }

    ImGui::EndFrame();
}

void UI::RenderUI()
{
    ImGui::DockSpaceOverViewport(ImGui::GetMainViewport()->ID);

    RenderMenuBar();
    RenderColorWindow();
    RenderToolWindow();
    RenderLayerWindow();

    if (sRenderSaveAsImgPopup) { RenderSaveAsImagePopup(); }
    if (sRenderSaveAsPrjPopup) { RenderSaveAsProjectPopup(); }
    if (sRenderSaveErrorPopup) { RenderSaveErrorPopup(); }
}

void UI::RenderNoProjectWindow()
{
    ImGui::DockSpaceOverViewport(ImGui::GetMainViewport()->ID);

    ImGui::Begin("No project window");

    if (ImGui::Selectable("New project")) { sRenderNewProjectPopup = true; }
    if (ImGui::Selectable("Open a project")) {}

    ImGui::End();

    if (sRenderNewProjectPopup) { RenderNewProjectPopup(); }
}

void UI::RenderDrawWindow(unsigned int framebuffer_texture_id,
                          const char* window_name)
{
    // TODO(scheph): A lot of code in this function does not need to run every
    // frame as it currenlty does. Fix that
    ImGui::Begin(window_name);
    sDrawWindowRendered = true;

    // ImGui window size
    float window_width = ImGui::GetContentRegionAvail().x;
    float window_height = ImGui::GetContentRegionAvail().y;

    // Screen position of the window
    ImVec2 pos = ImGui::GetCursorScreenPos();

    // Canvas' upper left and bottom right coordinates
    ImVec2 upper_left(pos.x, pos.y);
    ImVec2 bottom_right(pos.x + window_width, pos.y + window_height);

    if (Project::CanvasWidth() > Project::CanvasHeight())
    {
        // 'val_to_take' is the value we need to take from the lesser dimension
        // (width or height) to make the proportion of the framebuffer image
        // dimensions equal to the proportion of the canvas dimensions.

        // The value is taken from the lesser dimension since the greater one
        // goes from side to the opposite side

        // This is the equation: (window_height - val_to_take) / window_width =
        // canvas_height / canvas_width

        float val_to_take =
            (static_cast<float>(Project::CanvasHeight()) * window_width) /
                static_cast<float>(Project::CanvasWidth()) -
            window_height;

        // Taking a half from the top and adding a half to the bottom centers
        // the image
        upper_left.y -= val_to_take / 2;
        bottom_right.y += val_to_take / 2;
    }
    else
    {
        // The equation here is like this: (window_width - val_to_take) /
        // window_height = canvas_width / canvas_height

        float val_to_take =
            (static_cast<float>(Project::CanvasWidth()) * window_height) /
                static_cast<float>(Project::CanvasHeight()) -
            window_width;

        upper_left.x -= val_to_take / 2;
        bottom_right.x += val_to_take / 2;
    }

    ImGui::GetWindowDrawList()->AddImage(
        std::bit_cast<ImTextureID>(
            static_cast<uintptr_t>(framebuffer_texture_id)),
        upper_left, bottom_right);

    ImGui::End();

    GetDrawWinUpperleftCoords() = pos;
    GetDrawWinDimensions() = {window_width, window_height};
    GetCanvasUpperleftCoords() = upper_left;
    GetCanvasBottomRightCoords() = bottom_right;
}

void UI::Update()
{
    sDrawWindowRendered = false;
}

void UI::SetupToolTextures(std::span<unsigned int> tex_ids)
{
    auto index = 0UZ;

    for (auto tex_id : tex_ids)
    {
        sToolTextures.at(index) =
            std::bit_cast<ImTextureID>(static_cast<uintptr_t>(tex_id));
        index++;
    }

    /*sBrushToolTextureID =
        std::bit_cast<ImTextureID>(static_cast<uintptr_t>(brush_tex_id));
    sEraserToolTextureID =
        std::bit_cast<ImTextureID>(static_cast<uintptr_t>(eraser_tex_id));
    sColorPickerToolTextureID =
        std::bit_cast<ImTextureID>(static_cast<uintptr_t>(color_pick_tex_id));
    sBucketToolTextureID =
        std::bit_cast<ImTextureID>(static_cast<uintptr_t>(bucket_tex_id));*/
}

void UI::SetupLayerToolTextures(unsigned int eye_opened_id,
                                unsigned int eye_closed_id,
                                unsigned int lock_locked_id,
                                unsigned int lock_unlocked_id)
{
    sEyeOpenedTextureID =
        std::bit_cast<ImTextureID>(static_cast<uintptr_t>(eye_opened_id));
    sEyeClosedTextureID =
        std::bit_cast<ImTextureID>(static_cast<uintptr_t>(eye_closed_id));
    sLockLockedTextureID =
        std::bit_cast<ImTextureID>(static_cast<uintptr_t>(lock_locked_id));
    sLockUnlockedTextureID =
        std::bit_cast<ImTextureID>(static_cast<uintptr_t>(lock_unlocked_id));
}

auto UI::ShouldDoTool() -> bool
{
    return sShouldDoTool;
}

void UI::RenderMenuBar()
{
    ImGui::BeginMainMenuBar();

    if (ImGui::BeginMenu("File"))
    {
        if (ImGui::MenuItem("New"))
        {
            Project::CloseCurrentProject();
            sRenderNewProjectPopup = true;
        }
        if (ImGui::MenuItem("Save as image")) { sRenderSaveAsImgPopup = true; }
        if (ImGui::MenuItem("Save as project"))
        {
            sRenderSaveAsPrjPopup = true;
        }
        ImGui::EndMenu();
    }

    if (ImGui::BeginMenu("Edit"))
    {
        if (ImGui::MenuItem("Undo")) { Layers::Undo(); }
        if (ImGui::MenuItem("Redo")) { Layers::Redo(); }
        ImGui::EndMenu();
    }

    if (ImGui::BeginMenu("View"))
    {
        constexpr double kZoomAddVal = 0.1;
        if (ImGui::MenuItem("Zoom In")) { Camera::AddToZoom(kZoomAddVal); }
        if (ImGui::MenuItem("Zoom Out")) { Camera::AddToZoom(-kZoomAddVal); }
        if (ImGui::MenuItem("Reset Camera")) { Camera::ResetCamera(); }
        if (ImGui::MenuItem("Reset Center")) { Camera::ResetCenter(); }
        if (ImGui::MenuItem("Reset Zoom")) { Camera::ResetZoom(); }
        ImGui::EndMenu();
    }

    ImGui::EndMainMenuBar();
}

void UI::RenderSaveAsImagePopup()
{
    sShouldDoTool = false; // Don't want to draw with a popup opened

    static std::array<char, 256> destination_str;
    static std::array<char, 64> file_name_str;
    static int magnify_factor = 1;

    ImGui::OpenPopup("Save");

    if (ImGui::BeginPopupModal("Save", nullptr,
                               ImGuiWindowFlags_AlwaysAutoResize))
    {
        ImGui::Text("Destination:");
        ImGui::InputText("##dest_input", destination_str.data(),
                         destination_str.size());
        ImGui::Text("Picture name:");
        ImGui::InputText("##picname_input", file_name_str.data(),
                         file_name_str.size());
        ImGui::Text("Magnify factor:");
        ImGui::InputInt("##mag_input", &magnify_factor);

        if (ImGui::Button("Save"))
        {
            std::string destination(destination_str.data());
            destination += '/';
            destination += file_name_str.data();
            Project::SaveAsImage(magnify_factor, destination);
            sRenderSaveAsImgPopup = false;
            ImGui::CloseCurrentPopup();
        }

        ImGui::SameLine(0.0F, 10.0F);

        if (ImGui::Button("Cancel"))
        {
            sRenderSaveAsImgPopup = false;
            ImGui::CloseCurrentPopup();
        }

        ImGui::EndPopup();
    }
}

void UI::RenderSaveAsProjectPopup()
{
    sShouldDoTool = false; // Don't want to draw with a popup opened

    static std::array<char, 256> destination_str;
    static std::array<char, 64> file_name_str;
    static int magnify_factor = 1;

    ImGui::OpenPopup("Save");

    if (ImGui::BeginPopupModal("Save", nullptr,
                               ImGuiWindowFlags_AlwaysAutoResize))
    {
        ImGui::Text("Destination:");
        ImGui::InputText("##dest_input", destination_str.data(),
                         destination_str.size());
        ImGui::Text("Picture name:");
        ImGui::InputText("##picname_input", file_name_str.data(),
                         file_name_str.size());
        ImGui::Text("Magnify factor:");
        ImGui::InputInt("##mag_input", &magnify_factor);

        if (ImGui::Button("Save"))
        {
            std::string destination(destination_str.data());
            destination += '/';
            destination += file_name_str.data();

            Project::SaveAsProject(destination);

            sRenderSaveAsImgPopup = false;
            ImGui::CloseCurrentPopup();
        }

        ImGui::SameLine(0.0F, 10.0F);

        if (ImGui::Button("Cancel"))
        {
            sRenderSaveAsImgPopup = false;
            ImGui::CloseCurrentPopup();
        }

        ImGui::EndPopup();
    }
}

void UI::RenderColorWindow()
{
    ImGui::Begin("Color");
    ImGui::NewLine();

    ImGui::ColorPicker4("Current color", &(Tool::GetColorRef().x),
                        ImGuiColorEditFlags_NoAlpha);

    ImGui::NewLine();

    ImGuiColorEditFlags flags =
        ImGuiColorEditFlags_NoPicker | ImGuiColorEditFlags_NoInputs;

    int selected_color_slot = Tool::GetSelectedColorSlot();

    if (selected_color_slot == Tool::kColorSlot1) { BeginOutline(); }

    if (ImGui::ColorButton("Color 1", Tool::GetColor1(), flags))
    {
        Tool::SetCurrentColorToColor1();
    }

    if (selected_color_slot == Tool::kColorSlot1) { EndOutline(); }

    ImGui::SameLine(0.0F, 10.0F);
    ImGui::Text("Color 1");

    if (selected_color_slot == Tool::kColorSlot2) { BeginOutline(); }

    if (ImGui::ColorButton("Color 2", Tool::GetColor2(), flags))
    {
        Tool::SetCurrentColorToColor2();
    }

    if (selected_color_slot == Tool::kColorSlot2) { EndOutline(); }

    ImGui::SameLine(0.0F, 10.0F);
    ImGui::Text("Color 2");

    RenderColorPalette(Tool::GetColorRef());

    ImGui::End();
}

void UI::RenderColorPalette(ImVec4& color)
{
    // Generate a default palette. The palette will persist and can be edited.

    ImGui::NewLine();
    ImGui::NewLine();

    static bool saved_palette_init = true;
    /* static ImVec4 saved_palette[32] = {}; */
    static std::array<ImVec4, 32> saved_palette{};
    if (saved_palette_init)
    {
        for (std::size_t i = 0; i < saved_palette.size(); i++)
        {
            ImGui::ColorConvertHSVtoRGB(static_cast<float>(i) / 31.0F, 0.8F,
                                        0.8F, saved_palette.at(i).x,
                                        saved_palette.at(i).y,
                                        saved_palette.at(i).z);
            saved_palette.at(i).w = 1.0F; // Alpha
        }
        saved_palette_init = false;
    }

    ImGui::Text("Palette");
    for (std::size_t i = 0; i < saved_palette.size(); i++)
    {
        ImGui::PushID(static_cast<int>(i));

        if ((i % 8) != 0)
        {
            ImGui::SameLine(0.0F, ImGui::GetStyle().ItemSpacing.y);
        }

        ImGuiColorEditFlags palette_button_flags = ImGuiColorEditFlags_NoAlpha;
        if (ImGui::ColorButton("##palette", saved_palette.at(i),
                               palette_button_flags, ImVec2(20, 20)))
        {
            color = ImVec4(saved_palette.at(i).x, saved_palette.at(i).y,
                           saved_palette.at(i).z, color.w); // Preserve alpha!
        }

        // Allow user to drop colors into each palette entry. Note that
        // ColorButton() is already a drag source by default, unless specifying
        // the ImGuiColorEditFlags_NoDragDrop flag.
        if (ImGui::BeginDragDropTarget())
        {
            if (const ImGuiPayload* payload =
                    ImGui::AcceptDragDropPayload(IMGUI_PAYLOAD_TYPE_COLOR_3F))
            {
                memcpy(saved_palette.data(), payload->Data, sizeof(float) * 3);
            }
            if (const ImGuiPayload* payload =
                    ImGui::AcceptDragDropPayload(IMGUI_PAYLOAD_TYPE_COLOR_4F))
            {
                memcpy(saved_palette.data(), payload->Data, sizeof(float) * 4);
            }

            ImGui::EndDragDropTarget();
        }

        ImGui::PopID();
    }
}

void UI::RenderToolWindow()
{
    ImGui::Begin("Tools");
    ImGui::NewLine();

    ToolType tool_type = Tool::GetToolType();

    if (tool_type == kBrush) { BeginOutline(); }
    if (ImGui::ImageButton(sToolTextures[kBrush], {20.0F, 20.0F}))
    {
        Tool::SetToolType(kBrush);
    }
    if (tool_type == kBrush) { EndOutline(); }

    ImGui::SameLine(0.0F, 4.0F);

    if (tool_type == kEraser) { BeginOutline(); }
    if (ImGui::ImageButton(sToolTextures[kEraser], {20.0F, 20.0F}))
    {
        Tool::SetToolType(kEraser);
    }
    if (tool_type == kEraser) { EndOutline(); }

    ImGui::SameLine(0.0F, 4.0F);

    if (tool_type == kColorPicker) { BeginOutline(); }
    if (ImGui::ImageButton(sToolTextures[kColorPicker], {20.0F, 20.0F}))
    {
        Tool::SetToolType(kColorPicker);
    }
    if (tool_type == kColorPicker) { EndOutline(); }

    ImGui::SameLine(0.0F, 4.0F);

    if (tool_type == kBucket) { BeginOutline(); }
    if (ImGui::ImageButton(sToolTextures[kBucket], {20.0F, 20.0F}))
    {
        Tool::SetToolType(kBucket);
    }
    if (tool_type == kBucket) { EndOutline(); }

    ImGui::SameLine(0.0F, 4.0F);

    if (tool_type == kRectShape) { BeginOutline(); }
    if (ImGui::ImageButton(sToolTextures[kRectShape], {20.0F, 20.0F}))
    {
        Tool::SetToolType(kRectShape);
    }
    if (tool_type == kRectShape) { EndOutline(); }

    ImGui::NewLine();

    ImGui::PushItemWidth(200.0F);
    ImGui::SliderInt(" Brush size", &Tool::sBrushRadius, 1,
                     Project::CanvasWidth());
    ImGui::PopItemWidth();

    if (Tool::sBrushRadius < 1) { Tool::sBrushRadius = 1; }

    ImGui::End();
}

void UI::RenderLayerWindow()
{
    ImGui::Begin("Layers");

    if (ImGui::Button("Add a layer")) { Layers::AddLayer(); }

    auto layer_it = Layers::GetLayers().begin();
    for (std::size_t i = 0; i < Layers::GetLayers().size(); i++)
    {
        Layer& layer_traversed = *layer_it;
        layer_it++;

        // Many ImGui tools/widgets need a unique id to prevent some internal
        // ImGui conflicts and bugs
        std::string str_id_for_widgets = "Layer " + std::to_string(i + 1);

        ImTextureID visibility_tex =
            (layer_traversed.IsVisible() ? sEyeOpenedTextureID
                                         : sEyeClosedTextureID);
        ImTextureID lock_tex =
            (layer_traversed.IsLocked() ? sLockLockedTextureID
                                        : sLockUnlockedTextureID);

        ImGui::Separator();

        // Adding "_v" to the id so other widgets wouldn't have the same id
        // (which would be just str_id_for_widgets) Doing the same with other
        // widgets as well
        constexpr ImVec2 kImageButtonDims{14.0F, 14.0F};
        if (ImGui::ImageButton((str_id_for_widgets + "_v").c_str(),
                               visibility_tex, kImageButtonDims))
        {
            layer_traversed.SwitchVisibilityState();
        }

        ImGui::SameLine(0.0F, 1.0F);

        if (ImGui::ImageButton((str_id_for_widgets + "_l").c_str(), lock_tex,
                               kImageButtonDims))
        {
            layer_traversed.SwitchLockState();
        }

        bool this_is_selected_layer = (Layers::sCurrentLayerIndex == i);
        if (this_is_selected_layer) { BeginOutline(); }

        ImGui::SameLine(0.0F, 10.0F);
        if (ImGui::Button(layer_traversed.GetName().c_str(), {100.0F, 0.0F}))
        {
            Layers::sCurrentLayerIndex = i;
        }

        if (this_is_selected_layer) { EndOutline(); }

        ImGui::SameLine(0.0F, 10.0F);
        ImGui::PushItemWidth(100.0F);
        ImGui::PushID(static_cast<int>(i));

        if (ImGui::SliderInt("Opacity", &layer_traversed.mOpacity, 0, 255)) {}

        ImGui::PopID();
        ImGui::PopItemWidth();

        ImGui::SameLine(0.0F, 10.0F);
        if (ImGui::ArrowButton((str_id_for_widgets + "_abu").c_str(),
                               ImGuiDir_Up))
        {
            Layers::MoveUp(static_cast<int>(i));
        }

        ImGui::SameLine(0.0F, 1.0F);
        if (ImGui::ArrowButton((str_id_for_widgets + "_abd").c_str(),
                               ImGuiDir_Down))
        {
            Layers::MoveDown(static_cast<int>(i));
        }
    }

    RenderLayerWinContextMenu();

    ImGui::End();
}

// Don't forget to call this function before ImGui::End() as this function uses
// ImGui::IsWindowHovered()
void UI::RenderLayerWinContextMenu()
{
    // Code bellow renders popups for changing a layer's name
    static bool open_the_change_lay_name_popup = false;

    static std::string buff;

    if (ImGui::IsWindowHovered() &&
        ImGui::IsMouseClicked(ImGuiMouseButton_Right))
    {
        ImGui::OpenPopup("LayerPopup");
    }

    if (ImGui::BeginPopupContextItem("LayerPopup"))
    {
        if (ImGui::MenuItem("Change layer name"))
        {
            buff = Layers::GetCurrentLayer().mLayerName;
            open_the_change_lay_name_popup = true;
        }

        ImGui::EndPopup();
    }

    if (open_the_change_lay_name_popup)
    {
        ImGui::OpenPopup("Change layer name");
    }

    if (ImGui::BeginPopupModal("Change layer name", nullptr,
                               ImGuiWindowFlags_AlwaysAutoResize))
    {
        // Don't wanna edit canvas if this popup is opened
        sShouldDoTool = false;
        ImGui::InputText("##input", &buff);

        if (ImGui::Button("OK"))
        {
            if (buff.length() != 0)
            {
                Layers::GetCurrentLayer().mLayerName = buff;
            }

            ImGui::CloseCurrentPopup();
            open_the_change_lay_name_popup = false;
        }

        ImGui::SameLine(0.0F, 10.0F);

        if (ImGui::Button("Cancel"))
        {
            ImGui::CloseCurrentPopup();
            open_the_change_lay_name_popup = false;
        }

        ImGui::EndPopup();
    }
}

void UI::RenderSaveErrorPopup()
{
    sShouldDoTool = false; // Don't want to draw with a popup opened

    ImGui::OpenPopup("Error: Failed to save");

    if (ImGui::BeginPopupModal("Error: Failed to save", nullptr,
                               ImGuiWindowFlags_AlwaysAutoResize))
    {
        ImGui::Text("Failed to save the picture. Check your destination, "
                    "picture name and magnify factor");

        if (ImGui::Button("OK"))
        {
            sRenderSaveErrorPopup = false;
            ImGui::CloseCurrentPopup();
        }

        ImGui::EndPopup();
    }
}

void UI::RenderNewProjectPopup()
{
    sShouldDoTool = false;

    ImGui::OpenPopup("New project");

    if (ImGui::BeginPopupModal("New project", nullptr,
                               ImGuiWindowFlags_AlwaysAutoResize))
    {
        static int height = 32;
        static int width = 32;

        ImGui::Text("Insert height: ");
        ImGui::SameLine(0.0F, 5.0F);
        ImGui::SliderInt("##height", &height, 32, 2000);

        ImGui::Text("Insert width:  ");
        ImGui::SameLine(0.0F, 5.0F);
        ImGui::SliderInt("##width", &width, 32, 2000);

        if (ImGui::Button("OK"))
        {
            Project::New({width, height});
            sRenderNewProjectPopup = false;
        }

        ImGui::SameLine(0.0F, 5.0F);

        if (ImGui::Button("Cancel")) { sRenderNewProjectPopup = false; }

        ImGui::EndPopup();
    }
}

// The outline around a control. Don't forget to call EndOutline!
void UI::BeginOutline(ImVec4& outline_color /*= selected_item_outline_color*/)
{
    ImGui::GetStyle().FrameBorderSize = 1.0F;
    ImGui::PushStyleColor(ImGuiCol_Border, outline_color);
}

// The outline around a control
void UI::EndOutline()
{
    ImGui::GetStyle().FrameBorderSize = 0.0F;
    ImGui::PopStyleColor();
}
} // namespace Pikzel
