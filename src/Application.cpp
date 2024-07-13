#include "Application.hpp"
#include "Layer.hpp"

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <imgui_internal.h>

#include <GLFW/glfw3.h>

#include <iostream>

#include <stb/stb_image.h>
#define STBI_MSC_SECURE_CRT
#define STB_IMAGE_WRITE_IMPLEMENTATION
#define __STDC_LIB_EXT1__
#include <stb/stb_image_write.h>
#undef __STDC_LIB_EXT1__

namespace App
{
    void UI::ImGuiInit(GLFWwindow* _window)
    {
        s_Window = _window;

        // Setup Dear ImGui context
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();

        ImGuiIO& io = ImGui::GetIO();
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
        io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;         // Enable Docking
        io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;       // Enable Multi-Viewport / Platform Windows

        ImGui::StyleColorsDark();
        ImGui_ImplGlfw_InitForOpenGL(s_Window, true);
        ImGui_ImplOpenGL3_Init();

        s_SelectedItemOutlineColor = ImGui::GetStyleColorVec4(ImGuiCol_SliderGrab);
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
        // (Platform functions may change the current OpenGL context, so we save/restore it to make it easier to paste this code elsewhere.
        //  For this specific demo app we could also call glfwMakeContextCurrent(window) directly)
        if (ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
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

        RenderMainMenuBar();
        RenderColorWindow();
        RenderToolWindow();
        RenderLayerWindow();

        if (s_RenderSaveAsImgPopup)
            RenderSaveAsImagePopup();

        if (s_RenderSaveAsPrjPopup)
            RenderSaveAsProjectPopup();

        if (s_RenderSaveErrorPopup)
            RenderSaveErrorPopup();
	}

    void UI::RenderNoProjectWindow()
    {
        ImGui::DockSpaceOverViewport(ImGui::GetMainViewport()->ID);

        ImGui::Begin("No project window");

        if (ImGui::Selectable("New project"))
        {
            s_RenderNewProjectPopup = true;
        }

        if (ImGui::Selectable("Open a project"))
        {

        }

        ImGui::End();

        if (s_RenderNewProjectPopup)
            RenderNewProjectPopup();
    }

    void UI::RenderDrawWindow(unsigned int framebuffer_texture_id, const char* window_name)
    {
        ImGui::Begin(window_name);

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
            // 'val_to_take' is the value we need to take from the lesser dimension (width or height) to make
            // the proportion of the framebuffer image dimensions equal to the proportion of the canvas dimensions.

            // The value is taken from the lesser dimension since the greater one goes from side to the opposite side

            // This is the equation: (window_height - val_to_take) / window_width = canvas_height / canvas_width

            float val_to_take = (Project::CanvasHeight() * window_width) / (float)Project::CanvasWidth() - window_height;

            // Taking a half from the top and adding a half to the bottom centers the image
            upper_left.y -= val_to_take / 2;
            bottom_right.y += val_to_take / 2;
        }
        else
        {
            // The equation here is like this: (window_width - val_to_take) / window_height = canvas_width / canvas_height

            float val_to_take = (Project::CanvasWidth() * window_height) / (float)Project::CanvasHeight() - window_width;

            upper_left.x -= val_to_take / 2;
            bottom_right.x += val_to_take / 2;
        }

        ImGui::GetWindowDrawList()->AddImage(
            reinterpret_cast<ImTextureID>(static_cast<uintptr_t>(framebuffer_texture_id)),
            upper_left,
            bottom_right
        );

        ImGui::End();

        s_DrawWindowUpperleftCornerCoords.x = pos.x;
        s_DrawWindowUpperleftCornerCoords.y = pos.y;
        s_DrawWindowDimensions.x = window_width;
        s_DrawWindowDimensions.y = window_height;

        s_CanvasUpperleftCoords.x = upper_left.x;
        s_CanvasUpperleftCoords.y = upper_left.y;
        s_CanvasBottomrightCoords.x = bottom_right.x;
        s_CanvasBottomrightCoords.y = bottom_right.y;
    }

    void UI::SetupToolTextures(unsigned int brush_tex_id, unsigned int eraser_tex_id, unsigned int color_pick_tex_id, unsigned int bucket_tex_id)
    {
        s_BrushToolTextureID       = reinterpret_cast<ImTextureID>(static_cast<uintptr_t>(brush_tex_id));
        s_EraserToolTextureID      = reinterpret_cast<ImTextureID>(static_cast<uintptr_t>(eraser_tex_id));
        s_ColorPickerToolTextureID = reinterpret_cast<ImTextureID>(static_cast<uintptr_t>(color_pick_tex_id));
        s_BucketToolTextureID      = reinterpret_cast<ImTextureID>(static_cast<uintptr_t>(bucket_tex_id));
    }

    void UI::SetupLayerToolTextures(unsigned int eye_opened_id, unsigned int eye_closed_id, unsigned int lock_locked_id, unsigned int lock_unlocked_id)
    {
        s_EyeOpenedTextureID    = reinterpret_cast<ImTextureID>(static_cast<uintptr_t>(eye_opened_id));
        s_EyeClosedTextureID    = reinterpret_cast<ImTextureID>(static_cast<uintptr_t>(eye_closed_id));
        s_LockLockedTextureID   = reinterpret_cast<ImTextureID>(static_cast<uintptr_t>(lock_locked_id));
        s_LockUnlockedTextureID = reinterpret_cast<ImTextureID>(static_cast<uintptr_t>(lock_unlocked_id));
    }

    bool UI::ShouldUpdateVertexBuffer()
    {
        return s_UpdateVertexBuffer;
    }

    bool UI::ShouldDoTool()
    {
        return s_ShouldDoTool;
    }

    void UI::RenderMainMenuBar()
    {
        ImGui::BeginMainMenuBar();

        if (ImGui::BeginMenu("File"))
        {
            if (ImGui::MenuItem("New"))
            {
                Project::CloseCurrentProject();
                s_RenderNewProjectPopup = true;
            }

            if (ImGui::MenuItem("Save as image"))
            {
                s_RenderSaveAsImgPopup = true;
            }

            if (ImGui::MenuItem("Save as project"))
            {
                s_RenderSaveAsPrjPopup = true;
            }

            ImGui::EndMenu();
        }

        ImGui::EndMainMenuBar();
    }

    void UI::RenderSaveAsImagePopup()
    {
        s_ShouldDoTool = false;  // Don't want to draw with a popup opened

        static char destination_str[256];
        static char file_name_str[64];
        static int magnify_factor = 1;

        ImGui::OpenPopup("Save");

        if (ImGui::BeginPopupModal("Save", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
        {
            ImGui::Text("Destination:");
            ImGui::InputText("##dest_input", destination_str, IM_ARRAYSIZE(destination_str));
            ImGui::Text("Picture name:");
            ImGui::InputText("##picname_input", file_name_str, IM_ARRAYSIZE(file_name_str));
            ImGui::Text("Magnify factor:");
            ImGui::InputInt("##mag_input", &magnify_factor);

            if (ImGui::Button("Save"))
            {
                std::string destination = destination_str;
                destination += '/';
                destination += file_name_str;

                Project::SaveAsImage(magnify_factor, destination);

                s_RenderSaveAsImgPopup = false;
                ImGui::CloseCurrentPopup();
            }

            ImGui::SameLine(0.0f, 10.0f);

            if (ImGui::Button("Cancel"))
            {
                s_RenderSaveAsImgPopup = false;
                ImGui::CloseCurrentPopup();
            }

            ImGui::EndPopup();
        }
    }

    void UI::RenderSaveAsProjectPopup()
    {
        s_ShouldDoTool = false;  // Don't want to draw with a popup opened

        static char destination_str[256];
        static char file_name_str[64];

        ImGui::OpenPopup("Save");

        if (ImGui::BeginPopupModal("Save", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
        {
            ImGui::Text("Destination:");
            ImGui::InputText("##dest_input", destination_str, IM_ARRAYSIZE(destination_str));
            ImGui::Text("Picture name:");
            ImGui::InputText("##picname_input", file_name_str, IM_ARRAYSIZE(file_name_str));

            if (ImGui::Button("Save"))
            {
                std::string destination(destination_str);
                destination += '/';
                destination += file_name_str;

                Project::SaveAsProject(destination);

                s_RenderSaveAsPrjPopup = false;
                ImGui::CloseCurrentPopup();
            }

            ImGui::SameLine(0.0f, 10.0f);

            if (ImGui::Button("Cancel"))
            {
                s_RenderSaveAsPrjPopup = false;
                ImGui::CloseCurrentPopup();
            }

            ImGui::EndPopup();
        }
    }

    void UI::RenderColorWindow()
    {
        ImGui::Begin("Color");
        ImGui::NewLine();

        ImGui::ColorPicker4("Current color", &(Tool::GetColorRef().x), ImGuiColorEditFlags_NoAlpha);

        ImGui::NewLine();

        ImGuiColorEditFlags flags =
            ImGuiColorEditFlags_NoPicker | ImGuiColorEditFlags_NoInputs;

        int _selected_color_slot = Tool::GetSelectedColorSlot();

        if (_selected_color_slot == Tool::COLOR_SLOT_1)
            BeginOutline();

        if (ImGui::ColorButton("Color 1", Tool::GetColor1(), flags))
            Tool::SetCurrentColorToColor1();

        if (_selected_color_slot == Tool::COLOR_SLOT_1)
            EndOutline();

        ImGui::SameLine(0.0f, 10.0f);
        ImGui::Text("Color 1");

        if (_selected_color_slot == Tool::COLOR_SLOT_2)
            BeginOutline();

        if (ImGui::ColorButton("Color 2", Tool::GetColor2(), flags))
            Tool::SetCurrentColorToColor2();

        if (_selected_color_slot == Tool::COLOR_SLOT_2)
            EndOutline();

        ImGui::SameLine(0.0f, 10.0f);
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
        static ImVec4 saved_palette[32] = {};
        if (saved_palette_init)
        {
            for (int n = 0; n < IM_ARRAYSIZE(saved_palette); n++)
            {
                ImGui::ColorConvertHSVtoRGB(n / 31.0f, 0.8f, 0.8f,
                    saved_palette[n].x, saved_palette[n].y, saved_palette[n].z);
                saved_palette[n].w = 1.0f; // Alpha
            }
            saved_palette_init = false;
        }

        ImGui::Text("Palette");
        for (int n = 0; n < IM_ARRAYSIZE(saved_palette); n++)
        {
            ImGui::PushID(n);

            if ((n % 8) != 0)
                ImGui::SameLine(0.0f, ImGui::GetStyle().ItemSpacing.y);

            ImGuiColorEditFlags palette_button_flags = ImGuiColorEditFlags_NoAlpha;
            if (ImGui::ColorButton("##palette", saved_palette[n], palette_button_flags, ImVec2(20, 20)))
                color = ImVec4(saved_palette[n].x, saved_palette[n].y, saved_palette[n].z, color.w); // Preserve alpha!

            // Allow user to drop colors into each palette entry. Note that ColorButton() is already a
            // drag source by default, unless specifying the ImGuiColorEditFlags_NoDragDrop flag.
            if (ImGui::BeginDragDropTarget())
            {
                if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload(IMGUI_PAYLOAD_TYPE_COLOR_3F))
                    memcpy((float*)&saved_palette[n], payload->Data, sizeof(float) * 3);
                if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload(IMGUI_PAYLOAD_TYPE_COLOR_4F))
                    memcpy((float*)&saved_palette[n], payload->Data, sizeof(float) * 4);
                ImGui::EndDragDropTarget();
            }

            ImGui::PopID();
        }
    }

    void UI::RenderToolWindow()
    {
        ImGui::Begin("Tools");
        ImGui::NewLine();

        ToolType tool_type = Tool::GetToolType();  // Need to get the value here because it may change below

        /* BRUSH */
        if (tool_type == BRUSH)
            BeginOutline();

        if (ImGui::ImageButton(s_BrushToolTextureID, { 20.0f, 20.0f }))
            Tool::SetToolType(BRUSH);

        if (tool_type == BRUSH)
            EndOutline();

        /* ERASER */
        ImGui::SameLine(0.0f, 4.0f);

        if (tool_type == ERASER)
            BeginOutline();

        if (ImGui::ImageButton(s_EraserToolTextureID, { 20.0f, 20.0f }))
            Tool::SetToolType(ERASER);

        if (tool_type == ERASER)
            EndOutline();

        /* COLOR PICKER */
        ImGui::SameLine(0.0f, 4.0f);

        if (tool_type == COLOR_PICKER)
            BeginOutline();

        if (ImGui::ImageButton(s_ColorPickerToolTextureID, { 20.0f, 20.0f }))
            Tool::SetToolType(COLOR_PICKER);

        if (tool_type == COLOR_PICKER)
            EndOutline();

        /* BUCKET */
        ImGui::SameLine(0.0f, 4.0f);

        if (tool_type == BUCKET)
            BeginOutline();

        if (ImGui::ImageButton(s_BucketToolTextureID, { 20.0f, 20.0f }))
            Tool::SetToolType(BUCKET);

        if (tool_type == BUCKET)
            EndOutline();

        ImGui::NewLine();

        ImGui::PushItemWidth(100.0f);
        ImGui::InputInt(" Brush radius", &Tool::s_BrushRadius, 1, 10);
        ImGui::PopItemWidth();

        if (Tool::s_BrushRadius < 1) {
            Tool::s_BrushRadius = 1;
        }

        ImGui::End();
    }

    void UI::RenderLayerWindow()
    {
        ImGui::Begin("Layers");

        constexpr int NO_LAYER_RIGHT_CLICKED = -1;
        static int right_clicked_layer_index = NO_LAYER_RIGHT_CLICKED;

        if (ImGui::Button("Add a layer"))
            Layers::AddLayer();

        auto it = Layers::s_Layers.begin();

        for (int i = 0; i < Layers::s_Layers.size(); i++)
        {
            Layer& layer_traversed = *it;
            it++;

            // Many ImGui tools/widgets need a unique id to prevent some internal ImGui conflicts and bugs
            std::string str_id_for_widgets = "Layer " + std::to_string(i + 1);

            ImTextureID visibility_tex = (layer_traversed.IsVisible() ? s_EyeOpenedTextureID : s_EyeClosedTextureID);
            ImTextureID lock_tex = (layer_traversed.IsLocked() ? s_LockLockedTextureID : s_LockUnlockedTextureID);

            ImGui::Separator();

            // Adding "_v" to the id so other widgets would't have the same id (which would be just str_id_for_widgets)
            // Doing the same with other widgets as well
            if (ImGui::ImageButton((str_id_for_widgets + "_v").c_str(), visibility_tex, { 20.0f, 20.0f }))
            {
                s_UpdateVertexBuffer = true;
                layer_traversed.SwitchVisibilityState();
            }

            ImGui::SameLine(0.0f, 1.0f);

            if (ImGui::ImageButton((str_id_for_widgets + "_l").c_str(), lock_tex, { 20.0f, 20.0f }))
            {
                s_UpdateVertexBuffer = true;
                layer_traversed.SwitchLockState();
            }

            constexpr float item_height = 26.0f;

            // Checks if the layer traversed is the current selected layer. If so, draws borders around the layer button
            bool this_is_selected_layer = (Layers::s_CurrentLayerIndex == i);
            if (this_is_selected_layer)
                BeginOutline();

            ImGui::SameLine(0.0f, 10.0f);
            if (ImGui::Button(layer_traversed.GetName().c_str(), { 100.0f, item_height }))
                Layers::s_CurrentLayerIndex = i;

            if (this_is_selected_layer)
                EndOutline();

            if (right_clicked_layer_index == -1 && ImGui::IsItemHovered() && ImGui::IsMouseClicked(ImGuiMouseButton_Right))
                right_clicked_layer_index = i;

            ImGui::SameLine(0.0f, 10.0f);
            ImGui::PushItemWidth(100.0f);
            ImGui::PushID(i);
            
            if (ImGui::SliderInt("Opacity", &layer_traversed.m_Opacity, 0, 255))
                UI::SetVertexBuffUpdateToTrue();

            ImGui::PopID();
            ImGui::PopItemWidth();

            ImGui::SameLine(0.0f, 10.0f);
            if (ImGui::ArrowButton((str_id_for_widgets + "_abu").c_str(), ImGuiDir_Up))  // Move layer up button
            {
                s_UpdateVertexBuffer = true;  // Need to update the vb because the layer order changes
                Layers::MoveUp(i);
            }

            ImGui::SameLine(0.0f, 1.0f);
            if (ImGui::ArrowButton((str_id_for_widgets + "_abd").c_str(), ImGuiDir_Down))  // Move layer down button
            {
                s_UpdateVertexBuffer = true;
                Layers::MoveDown(i);
            }
        }

        // Code bellow renders popups for changing a layer's name
        static bool open_the_change_lay_name_popup = false;

        constexpr std::size_t BUFF_SIZE = 20;
        static char buff[BUFF_SIZE] = "";  // A buffer used for the input box in the popup for changing a layer's name

        if (right_clicked_layer_index != NO_LAYER_RIGHT_CLICKED && ImGui::IsMouseClicked(ImGuiMouseButton_Right))
            ImGui::OpenPopup("LayerPopup");

        if (ImGui::BeginPopup("LayerPopup"))
        {
            if (ImGui::MenuItem("Change layer name"))
            {
                // Setting buffer string to the name of the layer the user wants to change
                // so that string would be the string displayed when opening the popup
                const std::string& right_clicked_layer_name = Layers::AtIndex(right_clicked_layer_index).m_LayerName;
                strncpy_s(buff, right_clicked_layer_name.c_str(), std::min(BUFF_SIZE - 1, right_clicked_layer_name.size()));
                open_the_change_lay_name_popup = true;
            }

            ImGui::EndPopup();
        }

        if (open_the_change_lay_name_popup)
        {
            ImGui::OpenPopup("Change layer name");
        }

        if (ImGui::BeginPopupModal("Change layer name", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
        {
            s_ShouldDoTool = false;  // Don't wanna edit canvas if this popup is opened
            ImGui::InputText("##input", buff, IM_ARRAYSIZE(buff));

            if (ImGui::Button("OK"))
            {
                if (strlen(buff) != 0)
                    Layers::AtIndex(right_clicked_layer_index).m_LayerName = buff;

                ImGui::CloseCurrentPopup();
                open_the_change_lay_name_popup = false;
                right_clicked_layer_index = NO_LAYER_RIGHT_CLICKED;
            }

            ImGui::SameLine(0.0f, 10.0f);

            if (ImGui::Button("Cancel"))
            {
                ImGui::CloseCurrentPopup();
                open_the_change_lay_name_popup = false;
                right_clicked_layer_index = NO_LAYER_RIGHT_CLICKED;
            }

            ImGui::EndPopup();
        }

        ImGui::End();  // "Layers"
    }

    void UI::RenderSaveErrorPopup()
    {
        s_ShouldDoTool = false;  // Don't want to draw with a popup opened

        ImGui::OpenPopup("Error: Failed to save");

        if (ImGui::BeginPopupModal("Error: Failed to save", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
        {
            static int magnify_factor = 1;

            ImGui::Text("Failed to save the picture. Check your destination, picture name and magnify factor");

            if (ImGui::Button("OK"))
            {
                s_RenderSaveErrorPopup = false;
                ImGui::CloseCurrentPopup();
            }

            ImGui::EndPopup();
        }
    }

    void UI::RenderNewProjectPopup()
    {
        s_ShouldDoTool = false;

        ImGui::OpenPopup("New project");

        if (ImGui::BeginPopupModal("New project", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
        {
            static int height = 32;
            static int width  = 32;

            ImGui::Text("Insert height: ");
            ImGui::SameLine(0.0f, 5.0f);
            ImGui::SliderInt("##height", &height, 32, 2000);

            ImGui::Text("Insert width:  ");
            ImGui::SameLine(0.0f, 5.0f);
            ImGui::SliderInt("##width", &width, 32, 2000);

            if (ImGui::Button("OK"))
            {
                Project::New(height, width);
                s_RenderNewProjectPopup = false;
                s_UpdateVertexBuffer = true;
            }

            ImGui::SameLine(0.0f, 5.0f);

            if (ImGui::Button("Cancel"))
            {
                s_RenderNewProjectPopup = false;
            }

            ImGui::EndPopup();
        }
    }

    // Don't forget to call EndOutline!
    void UI::BeginOutline(ImVec4& outline_color /*= selected_item_outline_color*/)
    {
        ImGui::GetStyle().FrameBorderSize = 1.0f;
        ImGui::PushStyleColor(ImGuiCol_Border, outline_color);
    }

    void UI::EndOutline()
    {
        ImGui::GetStyle().FrameBorderSize = 0.0f;
        ImGui::PopStyleColor();
    }
}
