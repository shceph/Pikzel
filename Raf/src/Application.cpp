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
        window = _window;

        // Setup Dear ImGui context
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();

        ImGuiIO& io = ImGui::GetIO();
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
        io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;         // Enable Docking
        io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;       // Enable Multi-Viewport / Platform Windows

        ImGui::StyleColorsDark();
        ImGui_ImplGlfw_InitForOpenGL(window, true);
        ImGui_ImplOpenGL3_Init();

        selected_item_outline_color = ImGui::GetStyleColorVec4(ImGuiCol_SliderGrab);
    }

    void UI::Update()
    {
        should_do_tool = true;
        update_vertex_buffer = false;
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
        ImGui::DockSpaceOverViewport(ImGui::GetMainViewport());

        RenderMainMenuBar();
        RenderColorWindow();
        RenderToolWindow();
        RenderLayerWindow();

        if (render_save_error_popup)
            RenderSaveErrorPopup();
	}

    void UI::RenderDrawWindow(unsigned int framebuffer_texture_id, const char* window_name)
    {
        ImGui::Begin(window_name);

        // we access the ImGui window size
        float window_width = ImGui::GetContentRegionAvail().x;
        float window_height = ImGui::GetContentRegionAvail().y;

        if (window_width == 0.0f)
            window_width = 1.0f;
        if (window_height == 0.0f)
            window_height = 1.0f;

        float canvas_lenght = std::min(window_width, window_height);

        // we get the screen position of the window
        ImVec2 pos = ImGui::GetCursorScreenPos();

        float upper_left_x  = pos.x + (window_width - canvas_lenght) / 2;
        float upper_left_y = pos.y + (window_height - canvas_lenght) / 2;

        ImGui::GetWindowDrawList()->AddImage(
            (ImTextureID)framebuffer_texture_id,
            ImVec2(upper_left_x, upper_left_y),
            ImVec2(upper_left_x + canvas_lenght, upper_left_y + canvas_lenght)
        );

        //ImGui::PopStyleVar();
        ImGui::End();

        draw_window_upperleft_corner_coords.x = pos.x;
        draw_window_upperleft_corner_coords.y = pos.y;
        draw_window_dimensions.x = window_width;
        draw_window_dimensions.y = window_height;

        canvas_upperleft_coords.x = upper_left_x;
        canvas_upperleft_coords.y = upper_left_y;
        canvas_bottomright_coords.x = upper_left_x + canvas_lenght;
        canvas_bottomright_coords.y = upper_left_y + canvas_lenght;
    }

    void UI::SetupToolTextures(unsigned int brush_tex_id, unsigned int eraser_tex_id, unsigned int color_pick_tex_id, unsigned int bucket_tex_id)
    {
        brush_tool_texture_id           = (ImTextureID)brush_tex_id;
        eraser_tool_texture_id          = (ImTextureID)eraser_tex_id;
        color_picker_tool_texture_id    = (ImTextureID)color_pick_tex_id;
        bucket_tool_texture_id          = (ImTextureID)bucket_tex_id;
    }

    void UI::SetupLayerToolTextures(unsigned int eye_opened_id, unsigned int eye_closed_id, unsigned int lock_locked_id, unsigned int lock_unlocked_id)
    {
        eye_opened_texture_id       = (ImTextureID)eye_opened_id;
        eye_closed_texture_id       = (ImTextureID)eye_closed_id;
        lock_locked_texture_id      = (ImTextureID)lock_locked_id;
        lock_unlocked_texture_id    = (ImTextureID)lock_unlocked_id;
    }

    bool UI::ShouldUpdateVertexBuffer()
    {
        return update_vertex_buffer;
    }

    bool UI::ShouldDoTool()
    {
        return should_do_tool;
    }

    void UI::RenderMainMenuBar()
    {
        ImGui::BeginMainMenuBar();

        if (ImGui::BeginMenu("File"))
        {
            if (ImGui::MenuItem("Save"))
            {
                render_layer_save_popup = true;
            }

            ImGui::EndMenu();
        }

        ImGui::EndMainMenuBar();

        if (render_layer_save_popup)
            RenderLayerSavePopup();
    }

    void UI::RenderLayerSavePopup()
    {
        should_do_tool = false;  // Don't want to draw with a popup opened

        static char destination_str[256];
        static char file_name_str[64];

        ImGui::OpenPopup("Save");

        if (ImGui::BeginPopupModal("Save", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
        {
            static int magnify_factor = 1;

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

                Save(magnify_factor, destination);

                render_layer_save_popup = false;
                ImGui::CloseCurrentPopup();
            }

            ImGui::SameLine(0.0f, 10.0f);

            if (ImGui::Button("Cancel"))
            {
                render_layer_save_popup = false;
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
        {
            Tool::SetCurrentColorToColor1();
        }

        if (_selected_color_slot == Tool::COLOR_SLOT_1)
            EndOutline();

        ImGui::SameLine(0.0f, 10.0f);
        ImGui::Text("Color 1");

        if (_selected_color_slot == Tool::COLOR_SLOT_2)
            BeginOutline();

        if (ImGui::ColorButton("Color 2", Tool::GetColor2(), flags))
        {
            Tool::SetCurrentColorToColor2();
        }

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

        if (ImGui::ImageButton(brush_tool_texture_id, { 20.0f, 20.0f }))
            Tool::SetToolType(BRUSH);

        if (tool_type == BRUSH)
            EndOutline();

        /* ERASER */
        ImGui::SameLine(0.0f, 4.0f);

        if (tool_type == ERASER)
            BeginOutline();

        if (ImGui::ImageButton(eraser_tool_texture_id, { 20.0f, 20.0f }))
            Tool::SetToolType(ERASER);

        if (tool_type == ERASER)
            EndOutline();

        /* COLOR PICKER */
        ImGui::SameLine(0.0f, 4.0f);

        if (tool_type == COLOR_PICKER)
            BeginOutline();

        if (ImGui::ImageButton(color_picker_tool_texture_id, { 20.0f, 20.0f }))
            Tool::SetToolType(COLOR_PICKER);

        if (tool_type == COLOR_PICKER)
            EndOutline();

        /* BUCKET */
        ImGui::SameLine(0.0f, 4.0f);

        if (tool_type == BUCKET)
            BeginOutline();

        if (ImGui::ImageButton(bucket_tool_texture_id, { 20.0f, 20.0f }))
            Tool::SetToolType(BUCKET);

        if (tool_type == BUCKET)
            EndOutline();

        ImGui::NewLine();

        ImGui::PushItemWidth(100.0f);
        ImGui::InputInt(" Brush radius", &Tool::brush_radius, 1, 10);
        ImGui::PopItemWidth();

        if (Tool::brush_radius < 1) {
            Tool::brush_radius = 1;
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

        auto it = Layers::layers.begin();

        for (int i = 0; i < Layers::layers.size(); i++)
        {
            Layer& layer_traversed = *it;
            it++;

            // Many ImGui tools/widgets need a unique id to prevent some internal ImGui conflicts and bugs
            std::string str_id_for_widgets = "Layer " + std::to_string(i + 1);

            ImTextureID visibility_tex = (layer_traversed.IsVisible() ? eye_opened_texture_id : eye_closed_texture_id);
            ImTextureID lock_tex = (layer_traversed.IsLocked() ? lock_locked_texture_id : lock_unlocked_texture_id);

            ImGui::Separator();

            // Adding "_v" to the id so other widgets would't have the same id (which would be just str_id_for_widgets)
            // Doing the same with other widgets as well
            if (ImGui::ImageButton((str_id_for_widgets + "_v").c_str(), visibility_tex, { 20.0f, 20.0f }))
            {
                update_vertex_buffer = true;
                layer_traversed.SwitchVisibilityState();
            }

            ImGui::SameLine(0.0f, 1.0f);

            if (ImGui::ImageButton((str_id_for_widgets + "_l").c_str(), lock_tex, { 20.0f, 20.0f }))
            {
                update_vertex_buffer = true;
                layer_traversed.SwitchLockState();
            }

            constexpr float item_height = 26.0f;

            // Checks if the layer traversed is the current selected layer. If so, draws borders around the layer button
            bool this_is_selected_layer = (Layers::current_layer_index == i);
            if (this_is_selected_layer)
                BeginOutline();

            ImGui::SameLine(0.0f, 10.0f);
            if (ImGui::Button(layer_traversed.GetName().c_str(), { 100.0f, item_height }))
                Layers::current_layer_index = i;

            if (this_is_selected_layer)
                EndOutline();

            if (right_clicked_layer_index == -1 && ImGui::IsItemHovered() && ImGui::IsMouseClicked(ImGuiMouseButton_Right))
                right_clicked_layer_index = i;

            ImGui::SameLine(0.0f, 10.0f);
            ImGui::PushItemWidth(100.0f);
            ImGui::PushID(i);
            ImGui::SliderInt("Opacity", &layer_traversed.m_Opacity, 0, 255);
            ImGui::PopID();
            ImGui::PopItemWidth();

            ImGui::SameLine(0.0f, 10.0f);
            if (ImGui::ArrowButton((str_id_for_widgets + "_abu").c_str(), ImGuiDir_Up))  // Move layer up button
            {
                update_vertex_buffer = true;  // Need to update the vb because the layer order changes
                Layers::MoveUp(i);
            }

            ImGui::SameLine(0.0f, 1.0f);
            if (ImGui::ArrowButton((str_id_for_widgets + "_abd").c_str(), ImGuiDir_Down))  // Move layer down button
            {
                update_vertex_buffer = true;
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
            should_do_tool = false;  // Don't wanna edit canvas if this popup is opened
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
        should_do_tool = false;  // Don't want to draw with a popup opened

        ImGui::OpenPopup("Error: Failed to save");

        if (ImGui::BeginPopupModal("Error: Failed to save", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
        {
            static int magnify_factor = 1;

            ImGui::Text("Failed to save the picture. Check your destination, picture name and magnify factor");

            if (ImGui::Button("OK"))
            {
                render_save_error_popup = false;
                ImGui::CloseCurrentPopup();
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

    static bool SaveImageToPNG(const char* filename, int width, int height, int num_channels, const unsigned char* data)
    {
        return stbi_write_png(filename, width, height, num_channels, data, width * num_channels);
    }

    void Save(int magnify_factor, std::string save_dest)
    {
        constexpr int CHANNEL_COUNT = 4;
        int arr_height = CANVAS_HEIGHT * magnify_factor;
        int arr_width = CANVAS_HEIGHT * magnify_factor * CHANNEL_COUNT;
        std::vector<unsigned char> image_data(arr_height * arr_width);

        const CanvasData& canvas_displayed = Layers::GetDisplayedCanvas();
        
        for (int i = 0; i < CANVAS_HEIGHT; i++)
        {
            for (int j = 0; j < CANVAS_WIDTH; j++)
            {
                if (canvas_displayed[i][j].a > 1.0f)
                {
                    for (int k = 0; k < magnify_factor; k++)
                    {
                        for (int l = 0; l < magnify_factor; l++)
                        {
                            image_data[(i * magnify_factor + k) * arr_width + j * magnify_factor * CHANNEL_COUNT + l * CHANNEL_COUNT + 0] = (unsigned char)0;
                            image_data[(i * magnify_factor + k) * arr_width + j * magnify_factor * CHANNEL_COUNT + l * CHANNEL_COUNT + 1] = (unsigned char)0;
                            image_data[(i * magnify_factor + k) * arr_width + j * magnify_factor * CHANNEL_COUNT + l * CHANNEL_COUNT + 2] = (unsigned char)0;
                            image_data[(i * magnify_factor + k) * arr_width + j * magnify_factor * CHANNEL_COUNT + l * CHANNEL_COUNT + 3] = (unsigned char)0;
                        }
                    }

                    continue;
                }

                for (int k = 0; k < magnify_factor; k++)
                {
                    for (int l = 0; l < magnify_factor; l++)
                    {
                        image_data[(i * magnify_factor + k) * arr_width + j * magnify_factor * CHANNEL_COUNT + l * CHANNEL_COUNT + 0] = (unsigned char)(canvas_displayed[i][j].r * 255);
                        image_data[(i * magnify_factor + k) * arr_width + j * magnify_factor * CHANNEL_COUNT + l * CHANNEL_COUNT + 1] = (unsigned char)(canvas_displayed[i][j].g * 255);
                        image_data[(i * magnify_factor + k) * arr_width + j * magnify_factor * CHANNEL_COUNT + l * CHANNEL_COUNT + 2] = (unsigned char)(canvas_displayed[i][j].b * 255);
                        image_data[(i * magnify_factor + k) * arr_width + j * magnify_factor * CHANNEL_COUNT + l * CHANNEL_COUNT + 3] = (unsigned char)(canvas_displayed[i][j].a * 255);
                    }
                }
            }
        }

        int height = CANVAS_HEIGHT * magnify_factor;
        int width  = CANVAS_WIDTH * magnify_factor;

        if (!SaveImageToPNG(save_dest.c_str(), width, height, CHANNEL_COUNT, image_data.data()))
        {
            UI::render_save_error_popup = true;
        }
        
        //if (!stbi_write_png("saved_.png", width, height, 3, image_data.data(), 0))
        //{
        //    std::cout << "Failed saving the project!\n";
        //}

        //// Example image data
        //int width = 32;
        //int height = 32;
        //int num_channels = 4; // Assuming RGBA image
        //std::vector<unsigned char> imageData(width * height * num_channels, 255); // Example: all white image

        //// Save the image to a PNG file
        //SaveImageToPNG("output.png", width, height, num_channels, imageData.data());
    }
}