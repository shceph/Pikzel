#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#include <GL/glew.h>
#define GLFW_STATIC
#include <GLFW/glfw3.h>
#include <Gla/Gla.hpp>

#include <iostream>

#include <glm/gtc/matrix_transform.hpp>

static void glfwError(int id, const char* description)
{
    std::cout << "Glfw error: " << description << std::endl;
}

void RenderColorPalette(ImVec4& color)
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

    /*ImGuiColorEditFlags misc_flags =
        0;

    static ImVec4 backup_color;
    bool open_popup = ImGui::ColorButton("MyColor##3b", color, misc_flags);
    ImGui::SameLine(0, ImGui::GetStyle().ItemInnerSpacing.x);*/

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

    //open_popup |= ImGui::Button("Palette");
    //if (open_popup)
    //{
    //    ImGui::OpenPopup("mypicker");
    //    backup_color = color;
    //}
    //if (ImGui::BeginPopup("mypicker"))
    //{
    //    ImGui::Text("MY CUSTOM COLOR PICKER WITH AN AMAZING PALETTE!");
    //    ImGui::Separator();
    //    ImGui::ColorPicker4("##picker", (float*)&color, misc_flags | ImGuiColorEditFlags_NoSidePreview | ImGuiColorEditFlags_NoSmallPreview);
    //    ImGui::SameLine();

    //    ImGui::BeginGroup(); // Lock X position
    //    ImGui::Text("Current");
    //    ImGui::ColorButton("##current", color, ImGuiColorEditFlags_NoPicker | ImGuiColorEditFlags_AlphaPreviewHalf, ImVec2(60, 40));
    //    ImGui::Text("Previous");
    //    if (ImGui::ColorButton("##previous", backup_color, ImGuiColorEditFlags_NoPicker | ImGuiColorEditFlags_AlphaPreviewHalf, ImVec2(60, 40)))
    //        color = backup_color;
    //    ImGui::Separator();
    //    ImGui::Text("Palette");
    //    for (int n = 0; n < IM_ARRAYSIZE(saved_palette); n++)
    //    {
    //        ImGui::PushID(n);
    //        if ((n % 8) != 0)
    //            ImGui::SameLine(0.0f, ImGui::GetStyle().ItemSpacing.y);

    //        ImGuiColorEditFlags palette_button_flags = ImGuiColorEditFlags_NoAlpha | ImGuiColorEditFlags_NoPicker | ImGuiColorEditFlags_NoTooltip;
    //        if (ImGui::ColorButton("##palette", saved_palette[n], palette_button_flags, ImVec2(20, 20)))
    //            color = ImVec4(saved_palette[n].x, saved_palette[n].y, saved_palette[n].z, color.w); // Preserve alpha!

    //        // Allow user to drop colors into each palette entry. Note that ColorButton() is already a
    //        // drag source by default, unless specifying the ImGuiColorEditFlags_NoDragDrop flag.
    //        if (ImGui::BeginDragDropTarget())
    //        {
    //            if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload(IMGUI_PAYLOAD_TYPE_COLOR_3F))
    //                memcpy((float*)&saved_palette[n], payload->Data, sizeof(float) * 3);
    //            if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload(IMGUI_PAYLOAD_TYPE_COLOR_4F))
    //                memcpy((float*)&saved_palette[n], payload->Data, sizeof(float) * 4);
    //            ImGui::EndDragDropTarget();
    //        }

    //        ImGui::PopID();
    //    }
    //    ImGui::EndGroup();
    //    ImGui::EndPopup();
    //}
}

int main(int argc, const char* argv[])
{
    GLFWwindow* window;

    /* Initialize the library */
    if (!glfwInit())
        return -1;

    glfwSetErrorCallback(&glfwError);

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    int window_width = 1280;
    int window_height = 700;

    /* Create a windowed mode window and its OpenGL context */
    window = glfwCreateWindow(window_width, window_height, "PixelCraft", NULL, NULL);

    if (!window)
    {
        std::cout << "Failed to create window\n";
        glfwTerminate();
        return -1;
    }

    /* Make the window's context current */
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);

    glfwMaximizeWindow(window);

    if (glewInit() != GLEW_OK)
    {
        std::cout << "Glew init error\n";
    }

    std::cout << glGetString(GL_VERSION) << '\n';

    GLCall(glEnable(GL_DEPTH_TEST));
    GLCall(glDepthFunc(GL_GREATER));

    GLCall(glEnable(GL_BLEND));
    GLCall(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;         // Enable Docking
    io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;       // Enable Multi-Viewport / Platform Windows

    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init();

    {  // Made this block so all the OpenGL objects would get destroyed before calling glfwTerminate.
        
        /* Loop until the user closes the window */
        while (!glfwWindowShouldClose(window))
        {
            glfwGetWindowSize(window, &window_width, &window_height);

            glViewport(0, 0, window_width, window_height);

            /* ImGui */
            ImGui_ImplOpenGL3_NewFrame();
            ImGui_ImplGlfw_NewFrame();
            ImGui::NewFrame();

            ImGui::DockSpaceOverViewport(ImGui::GetMainViewport());

            ImGui::Begin("Brush");
            static ImVec4 color1 = { 0.0f, 0.0f, 0.0f, 1.0f };
            static ImVec4 color2 = { 0.0f, 0.0f, 0.0f, 1.0f };
            static ImVec4* color = &color1;
            ImGui::ColorPicker4("Pick a color", &(color->x));

            ImGui::NewLine();

            ImGuiColorEditFlags flags =
                ImGuiColorEditFlags_NoPicker | ImGuiColorEditFlags_NoInputs;

            if (ImGui::ColorButton("Color 1", color1, flags))
            {
                color = &color1;
            }

            ImGui::SameLine(0.0f, 10.0f);
            ImGui::Text("Color 1");

            if (ImGui::ColorButton("Color 2", color2, flags))
            {
                color = &color2;
            }

            ImGui::SameLine(0.0f, 10.0f);
            ImGui::Text("Color 2");

            RenderColorPalette(*color);

            ImGui::End();

            //ImGui::ShowDemoWindow();

            ImGui::Render();
            ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

            ImGui::EndFrame();

            // Update and Render additional Platform Windows
            // (Platform functions may change the current OpenGL context, so we save/restore it to make it easier to paste this code elsewhere.
            //  For this specific demo app we could also call glfwMakeContextCurrent(window) directly)
            if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
            {
                GLFWwindow* backup_current_context = glfwGetCurrentContext();
                ImGui::UpdatePlatformWindows();
                ImGui::RenderPlatformWindowsDefault();
                glfwMakeContextCurrent(backup_current_context);
            }

            /* Swap front and back buffers */
            glfwSwapBuffers(window);

            /* Poll for and process events */
            glfwPollEvents();
        }

    }

    glfwTerminate();
    return 0;
}

/* Triangle code:

        Gla::Renderer renderer;

        float triangle_vertices[] = {
            -0.5f, -0.5f,  0.0f, 0.0f, 1.0f,
             0.0f,  0.5f,  0.0f, 1.0f, 0.0f,
             0.5f, -0.5f,  1.0f, 0.0f, 0.0f
        };

        Gla::VertexArray va;
        Gla::VertexBuffer vb(triangle_vertices, sizeof(triangle_vertices));
        Gla::VertexBufferLayout layout;
        layout.Push<float>(2);
        layout.Push<float>(3);
        va.AddBuffer(vb, layout);
        Gla::Shader shader("shader/VertShader.vert", "shader/FragShader.frag");
        Gla::Mesh mesh(va, shader);
        mesh.Bind();

        {
            renderer.Clear();
            renderer.DrawArrays(GL_TRIANGLES, sizeof triangle_vertices / sizeof(float));
        }
*/