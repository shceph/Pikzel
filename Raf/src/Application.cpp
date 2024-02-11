#include "Application.hpp"

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#include <GLFW/glfw3.h>

#include <iostream>

namespace App
{
    void UI::ImGuiInit(GLFWwindow* _window)
    {
        window = _window;

        // Setup Dear ImGui context
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
        ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
        ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_DockingEnable;         // Enable Docking
        ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;       // Enable Multi-Viewport / Platform Windows

        ImGui::StyleColorsDark();
        ImGui_ImplGlfw_InitForOpenGL(window, true);
        ImGui_ImplOpenGL3_Init();
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

        ImGui::BeginMainMenuBar();
        ImGui::EndMainMenuBar();

        RenderBrushWindow();
	}

    void UI::RenderDrawWindow(unsigned int framebuffer_texture_id, const char* window_name)
    {
        ImGui::Begin(window_name);

        // TODO: For some reason, the function below doesn't set the minimum size
        // should check it out
        ImGui::PushStyleVar(ImGuiStyleVar_WindowMinSize, ImVec2(100.0f, 100.0f));

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

        ImGui::PopStyleVar();
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

    void UI::RenderBrushWindow()
    {
        ImGui::Begin("Brush");
        ImGui::ColorPicker4("Current color", &(Brush::GetColorRef().x));

        ImGui::NewLine();

        ImGuiColorEditFlags flags =
            ImGuiColorEditFlags_NoPicker | ImGuiColorEditFlags_NoInputs;

        if (ImGui::ColorButton("Color 1", Brush::GetColor1(), flags))
        {
            Brush::SetCurrentColorToColor1();
        }

        ImGui::SameLine(0.0f, 10.0f);
        ImGui::Text("Color 1");

        if (ImGui::ColorButton("Color 2", Brush::GetColor2(), flags))
        {
            Brush::SetCurrentColorToColor2();
        }

        ImGui::SameLine(0.0f, 10.0f);
        ImGui::Text("Color 2");

        RenderColorPalette(Brush::GetColorRef());

        ImGui::NewLine();
        ImGui::NewLine();

        ImGui::SliderInt("Brush radius", &Brush::brush_radius, 1, 10);

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
}