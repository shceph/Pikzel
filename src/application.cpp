#include "application.hpp"
#include "camera.hpp"
#include "layer.hpp"
#include "tool.hpp"

#include <cstddef>
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
UI::UI(Project& project, Tool& tool, GLFWwindow* _window)
    : mTool(tool), mProject(project)
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
    /* ImGui::StyleColorsClassic(); */
    /* ImGui::StyleColorsLight(); */
    ImGui_ImplGlfw_InitForOpenGL(sWindow, true);
    ImGui_ImplOpenGL3_Init("#version 330");

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

    sConstructCounter++;
    assert(sConstructCounter < 2);
}

void UI::NewFrame()
{
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    ImGui::DockSpaceOverViewport(ImGui::GetMainViewport()->ID, nullptr);
}

void UI::RenderAndEndFrame()
{
    ImGui::Render();
    ImVec4 clear_color = ImVec4(0.8F, 0.8F, 0.8F, 1.00F);
    int display_w = 0;
    int display_h = 0;
    glfwGetFramebufferSize(sWindow, &display_w, &display_h);
    glViewport(0, 0, display_w, display_h);
    glClearColor(clear_color.x * clear_color.w, clear_color.y * clear_color.w,
                 clear_color.z * clear_color.w, clear_color.w);
    glClear(GL_COLOR_BUFFER_BIT);
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
}

void UI::RenderUI(Layers& layers, Camera& camera)
{
    RenderMenuBar(layers, camera);
    RenderColorWindow();
    RenderToolWindow();
    RenderLayerWindow(layers);
    RenderUndoTreeWindow(layers);

    if (mRenderSaveAsImgPopup) { RenderSaveAsImagePopup(); }
    if (mRenderSaveAsPrjPopup) { RenderSaveAsProjectPopup(); }
    if (mRenderSaveErrorPopup) { RenderSaveErrorPopup(); }
}

void UI::RenderNoProjectWindow()
{
    ImGui::Begin("No project window");

    if (ImGui::Selectable("New project")) { mRenderNewProjectPopup = true; }
    if (ImGui::Selectable("Open a project")) { mRenderOpenProjectPopup = true; }

    ImGui::End();

    if (mRenderNewProjectPopup) { RenderNewProjectPopup(); }
    if (mRenderOpenProjectPopup) { RenderOpenProjectPopup(); }
}

void UI::RenderDrawWindow(unsigned int framebuffer_texture_id,
                          const char* window_name)
{
    // TODO: A lot of code in this function does not need to run every
    // frame as it currenlty does. Fix that
    ImGui::Begin(window_name);
    mDrawWindowRendered = true;

    // ImGui window size
    float window_width = ImGui::GetContentRegionAvail().x;
    float window_height = ImGui::GetContentRegionAvail().y;

    // Screen position of the window
    ImVec2 pos = ImGui::GetCursorScreenPos();

    // Canvas' upper left and bottom right coordinates
    ImVec2 upper_left(pos.x, pos.y);
    ImVec2 bottom_right(pos.x + window_width, pos.y + window_height);

    if (mProject.get().CanvasWidth() > mProject.get().CanvasHeight())
    {
        // 'val_to_take' is the value we need to take from the lesser dimension
        // (width or height) to make the proportion of the framebuffer image
        // dimensions equal to the proportion of the canvas dimensions.

        // The value is taken from the lesser dimension since the greater one
        // goes from side to the opposite side

        // This is the equation: (window_height - val_to_take) / window_width =
        // canvas_height / canvas_width

        float val_to_take =
            ((static_cast<float>(mProject.get().CanvasHeight()) *
              window_width) /
             static_cast<float>(mProject.get().CanvasWidth())) -
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
            ((static_cast<float>(mProject.get().CanvasWidth()) *
              window_height) /
             static_cast<float>(mProject.get().CanvasHeight())) -
            window_width;

        upper_left.x -= val_to_take / 2;
        bottom_right.x += val_to_take / 2;
    }

    ImGui::GetWindowDrawList()->AddImage(
        std::bit_cast<ImTextureID>(
            static_cast<uintptr_t>(framebuffer_texture_id)),
        upper_left, bottom_right);

    ImGui::End();

    mDrawWinDimensions = ImVec2{window_width, window_height};
    GetCanvasUpperleftCoordsRef() = upper_left;
    GetCanvasBottomRightCoordsRef() = bottom_right;
}

void UI::Update()
{
    mDrawWindowRendered = false;
}

void UI::SetupToolTextures(std::span<unsigned int> tex_ids)
{
    auto index = 0UZ;

    for (auto tex_id : tex_ids)
    {
        mToolTextures.at(index) =
            std::bit_cast<ImTextureID>(static_cast<uintptr_t>(tex_id));
        index++;
    }
}

void UI::SetupLayerToolTextures(std::span<unsigned int> layer_tex_ids)
{
    mEyeOpenedTextureID = layer_tex_ids[0];
    mEyeClosedTextureID = layer_tex_ids[1];
    mLockLockedTextureID = layer_tex_ids[2];
    mLockUnlockedTextureID = layer_tex_ids[3];
}

auto UI::ShouldDoTool() const -> bool
{
    return mShouldDoTool;
}

void UI::RenderMenuBar(Layers& layers, Camera& camera)
{
    ImGui::BeginMainMenuBar();

    if (ImGui::BeginMenu("File"))
    {
        if (ImGui::MenuItem("New"))
        {
            mProject.get().CloseCurrentProject();
            mRenderNewProjectPopup = true;
        }
        if (ImGui::MenuItem("Save as image")) { mRenderSaveAsImgPopup = true; }
        if (ImGui::MenuItem("Save as project"))
        {
            mRenderSaveAsPrjPopup = true;
        }
        ImGui::EndMenu();
    }

    if (ImGui::BeginMenu("Edit"))
    {
        if (ImGui::MenuItem("Undo")) { layers.MarkForUndo(); }
        if (ImGui::MenuItem("Redo")) { layers.MarkForRedo(); }
        if (ImGui::MenuItem("Undo Tree")) { mRenderUndoTreeWindow = true; }
        ImGui::EndMenu();
    }

    if (ImGui::BeginMenu("View"))
    {
        constexpr double kZoomAddVal = 0.1;
        if (ImGui::MenuItem("Zoom In")) { camera.AddToZoom(kZoomAddVal); }
        if (ImGui::MenuItem("Zoom Out")) { camera.AddToZoom(-kZoomAddVal); }
        if (ImGui::MenuItem("Reset Camera")) { camera.ResetCamera(); }
        if (ImGui::MenuItem("Reset Center")) { camera.ResetCenter(); }
        if (ImGui::MenuItem("Reset Zoom")) { camera.ResetZoom(); }
        ImGui::EndMenu();
    }

    ImGui::EndMainMenuBar();
}

void UI::RenderSaveAsImagePopup()
{
    mShouldDoTool = false; // Don't want to draw with a popup opened

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

            if (!mProject.get().SaveAsImage(magnify_factor, destination))
            {
                TriggerSaveErrorPopup();
            }

            mRenderSaveAsImgPopup = false;
            ImGui::CloseCurrentPopup();
        }

        ImGui::SameLine(0.0F, 10.0F);

        if (ImGui::Button("Cancel"))
        {
            mRenderSaveAsImgPopup = false;
            ImGui::CloseCurrentPopup();
        }

        ImGui::EndPopup();
    }
}

void UI::RenderSaveAsProjectPopup()
{
    mShouldDoTool = false;

    static std::array<char, 256> destination_str;
    static std::array<char, 64> file_name_str;

    ImGui::OpenPopup("Save");

    if (ImGui::BeginPopupModal("Save", nullptr,
                               ImGuiWindowFlags_AlwaysAutoResize))
    {
        ImGui::Text("Destination:");
        ImGui::InputText("##dest_input", destination_str.data(),
                         destination_str.size());
        ImGui::Text("File name:");
        ImGui::InputText("##filename_input", file_name_str.data(),
                         file_name_str.size());

        if (ImGui::Button("Save"))
        {
            std::string destination(destination_str.data());
            destination += '/';
            destination += file_name_str.data();
            mProject.get().SaveAsProject(destination);
            mRenderSaveAsPrjPopup = false;
            ImGui::CloseCurrentPopup();
        }

        ImGui::SameLine(0.0F, 10.0F);

        if (ImGui::Button("Cancel"))
        {
            mRenderSaveAsPrjPopup = false;
            ImGui::CloseCurrentPopup();
        }

        ImGui::EndPopup();
    }
}

void UI::RenderColorWindow()
{
    ImGui::Begin("Color");
    ImGui::NewLine();

    ImGui::ColorPicker4("Current color",
                        std::bit_cast<float*>(&mTool.get().GetColorRef()),
                        ImGuiColorEditFlags_NoAlpha);

    ImGui::NewLine();

    ImGuiColorEditFlags flags =
        ImGuiColorEditFlags_NoPicker | ImGuiColorEditFlags_NoInputs;

    int selected_color_slot = mTool.get().GetSelectedColorSlot();

    if (selected_color_slot == Tool::kColorSlot1) { BeginOutline(); }

    if (ImGui::ColorButton("Color 1", mTool.get().GetColor1(), flags))
    {
        mTool.get().SetCurrentColorToColor1();
    }

    if (selected_color_slot == Tool::kColorSlot1) { EndOutline(); }

    ImGui::SameLine(0.0F, 10.0F);
    ImGui::Text("Color 1");

    if (selected_color_slot == Tool::kColorSlot2) { BeginOutline(); }

    if (ImGui::ColorButton("Color 2", mTool.get().GetColor2(), flags))
    {
        mTool.get().SetCurrentColorToColor2();
    }

    if (selected_color_slot == Tool::kColorSlot2) { EndOutline(); }

    ImGui::SameLine(0.0F, 10.0F);
    ImGui::Text("Color 2");

    RenderColorPalette(mTool.get().GetColorRef());

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

void UI::RenderNodesChildren(Layers& layers, Tree<Layers::Capture>& node)
{
    mRenderNodesChildrenFuncData.node_count++;
    const auto& children = node.GetChildren();
    bool is_current_node = &layers.GetCurrentUndoTreeNode() == &node;
    bool has_multiple_children = children.size() > 1;

    std::string node_id =
        "Node" + std::to_string(mRenderNodesChildrenFuncData.node_count);
    const char* curr = is_current_node ? " - Current Node" : "";

    int lifetime_in_sec =
        static_cast<int>(glfwGetTime()) - node.GetData().time_of_creation;
    int lifetime_in_min = lifetime_in_sec / 60;
    const char* min_ago = lifetime_in_min == 1 ? "minute ago" : "minutes ago";

    ImGui::SetNextItemOpen(true, 1);

    if (ImGui::TreeNodeEx(node_id.c_str(), ImGuiTreeNodeFlags_None, "%d %s%s",
                          lifetime_in_min, min_ago, curr))
    {
        if (ImGui::IsItemClicked(ImGuiMouseButton_Left))
        {
            mRenderNodesChildrenFuncData.clicked_node = &node;
        }

        if (has_multiple_children)
        {
            for (std::size_t i = 1; i < children.size(); i++)
            {
                RenderNodesChildren(layers, *children[i]);
            }
        }

        ImGui::TreePop();
    }

    if (has_multiple_children)
    {
        RenderNodesChildren(layers, *children.front());
        return;
    }
    if (children.size() == 0) { return; }

    RenderNodesChildren(layers, *children.front());
}

void UI::RenderUndoTreeWindow(Layers& layers)
{
    if (!mRenderUndoTreeWindow) { return; }

    ImGui::Begin("Undo Tree", &mRenderUndoTreeWindow, ImGuiWindowFlags_None);
    mShouldDoTool = false;

    mRenderNodesChildrenFuncData.Reset();
    RenderNodesChildren(layers, layers.GetUndoTree());

    ImGui::End();

    if (mRenderNodesChildrenFuncData.clicked_node != nullptr)
    {
        layers.SetCurrentNode(*mRenderNodesChildrenFuncData.clicked_node);
    }
}

void UI::RenderToolWindow()
{
    ImGui::Begin("Tools");
    ImGui::NewLine();

    ToolType tool_type = mTool.get().GetToolType();

    if (tool_type == ToolType::kBrush) { BeginOutline(); }
    if (ImGui::ImageButton(
            "ib1", mToolTextures[static_cast<std::size_t>(ToolType::kBrush)],
            {20.0F, 20.0F}))
    {
        mTool.get().SetToolType(ToolType::kBrush);
    }
    if (tool_type == ToolType::kBrush) { EndOutline(); }

    ImGui::SameLine(0.0F, 4.0F);

    if (tool_type == ToolType::kEraser) { BeginOutline(); }
    if (ImGui::ImageButton(
            "ib2", mToolTextures[static_cast<std::size_t>(ToolType::kEraser)],
            {20.0F, 20.0F}))
    {
        mTool.get().SetToolType(ToolType::kEraser);
    }
    if (tool_type == ToolType::kEraser) { EndOutline(); }

    ImGui::SameLine(0.0F, 4.0F);

    if (tool_type == ToolType::kColorPicker) { BeginOutline(); }
    if (ImGui::ImageButton(
            "ib3",
            mToolTextures[static_cast<std::size_t>(ToolType::kColorPicker)],
            {20.0F, 20.0F}))
    {
        mTool.get().SetToolType(ToolType::kColorPicker);
    }
    if (tool_type == ToolType::kColorPicker) { EndOutline(); }

    ImGui::SameLine(0.0F, 4.0F);

    if (tool_type == ToolType::kBucket) { BeginOutline(); }
    if (ImGui::ImageButton(
            "ib4", mToolTextures[static_cast<std::size_t>(ToolType::kBucket)],
            {20.0F, 20.0F}))
    {
        mTool.get().SetToolType(ToolType::kBucket);
    }
    if (tool_type == ToolType::kBucket) { EndOutline(); }

    ImGui::SameLine(0.0F, 4.0F);

    if (tool_type == ToolType::kRectShape) { BeginOutline(); }
    if (ImGui::ImageButton(
            "ib5",
            mToolTextures[static_cast<std::size_t>(ToolType::kRectShape)],
            {20.0F, 20.0F}))
    {
        mTool.get().SetToolType(ToolType::kRectShape);
    }
    if (tool_type == ToolType::kRectShape) { EndOutline(); }

    ImGui::NewLine();

    ImGui::PushItemWidth(200.0F);
    ImGui::SliderInt(" Brush size", &mTool.get().mBrushRadius, 1,
                     mProject.get().CanvasWidth());
    ImGui::PopItemWidth();

    if (mTool.get().GetBrushRadius() < 1) { mTool.get().SetBrushRadius(1); }

    ImGui::End();
}

void UI::RenderLayerWindow(Layers& layers)
{
    ImGui::Begin("Layers");

    if (ImGui::Button("Add a layer")) { layers.MarkToAddLayer(); }

    auto layer_it = layers.GetLayers().begin();
    for (std::size_t i = 0; i < layers.GetLayers().size(); i++)
    {
        Layer& layer_traversed = *layer_it;
        layer_it++;

        // Many ImGui tools/widgets need a unique id to prevent some internal
        // ImGui conflicts and bugs
        std::string str_id_for_widgets = "Layer " + std::to_string(i + 1);

        ImTextureID visibility_tex =
            (layer_traversed.IsVisible() ? mEyeOpenedTextureID
                                         : mEyeClosedTextureID);
        ImTextureID lock_tex =
            (layer_traversed.IsLocked() ? mLockLockedTextureID
                                        : mLockUnlockedTextureID);

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

        bool this_is_selected_layer = (layers.mCurrentLayerIndex == i);
        if (this_is_selected_layer) { BeginOutline(); }

        ImGui::SameLine(0.0F, 10.0F);
        if (ImGui::Button(layer_traversed.GetName().c_str(), {100.0F, 0.0F}))
        {
            layers.mCurrentLayerIndex = i;
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
            layers.MoveUp(static_cast<int>(i));
        }

        ImGui::SameLine(0.0F, 1.0F);
        if (ImGui::ArrowButton((str_id_for_widgets + "_abd").c_str(),
                               ImGuiDir_Down))
        {
            layers.MoveDown(static_cast<int>(i));
        }
    }

    RenderLayerWinContextMenu(layers);

    ImGui::End();
}

// Don't forget to call this function before ImGui::End() as this function uses
// ImGui::IsWindowHovered()
void UI::RenderLayerWinContextMenu(Layers& layers)
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
            buff = layers.GetCurrentLayer().mLayerName;
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
        mShouldDoTool = false;
        ImGui::InputText("##input", &buff);

        if (ImGui::Button("OK"))
        {
            if (buff.length() != 0)
            {
                layers.GetCurrentLayer().mLayerName = buff;
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
    mShouldDoTool = false; // Don't want to draw with a popup opened

    ImGui::OpenPopup("Error: Failed to save");

    if (ImGui::BeginPopupModal("Error: Failed to save", nullptr,
                               ImGuiWindowFlags_AlwaysAutoResize))
    {
        ImGui::Text("Failed to save the picture. Check your destination, "
                    "picture name and magnify factor");

        if (ImGui::Button("OK"))
        {
            mRenderSaveErrorPopup = false;
            ImGui::CloseCurrentPopup();
        }

        ImGui::EndPopup();
    }
}

void UI::RenderNewProjectPopup()
{
    mShouldDoTool = false;

    ImGui::OpenPopup("New project");

    if (ImGui::BeginPopupModal("New project", nullptr,
                               ImGuiWindowFlags_AlwaysAutoResize))
    {
        static int height = 32;
        static int width = 32;

        ImGui::Text("Insert height: ");
        ImGui::SameLine(0.0F, 5.0F);
        ImGui::SliderInt("##height", &height, 32, 2048);

        ImGui::Text("Insert width:  ");
        ImGui::SameLine(0.0F, 5.0F);
        ImGui::SliderInt("##width", &width, 32, 2048);

        if (ImGui::Button("OK"))
        {
            mProject.get().New({width, height});
            mRenderNewProjectPopup = false;
        }

        ImGui::SameLine(0.0F, 5.0F);

        if (ImGui::Button("Cancel")) { mRenderNewProjectPopup = false; }

        ImGui::EndPopup();
    }
}

void UI::RenderOpenProjectPopup()
{
    static std::array<char, 256> destination_str;

    ImGui::OpenPopup("Open");

    if (ImGui::BeginPopupModal("Open", nullptr,
                               ImGuiWindowFlags_AlwaysAutoResize))
    {
        ImGui::Text("Destination:");
        ImGui::InputText("##dest_input", destination_str.data(),
                         destination_str.size());

        if (ImGui::Button("Open"))
        {
            std::string destination(destination_str.data());
            mProject.get().Open(destination);
            mRenderOpenProjectPopup = false;
            ImGui::CloseCurrentPopup();
        }

        ImGui::SameLine(0.0F, 10.0F);

        if (ImGui::Button("Cancel"))
        {
            mRenderOpenProjectPopup = false;
            ImGui::CloseCurrentPopup();
        }

        ImGui::EndPopup();
    }
}

// The outline around a control. Don't forget to call EndOutline!
void UI::BeginOutline(
    ImVec4 outline_color /*= ImGui::GetStyleColorVec4(ImGuiCol_SliderGrab)*/)
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
