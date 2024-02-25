#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#define GLEW_STATIC
#include <GL/glew.h>
#define GLFW_STATIC
#include <GLFW/glfw3.h>
#include <Gla/Gla.hpp>

#include <iostream>
#include <vector>
#include <memory>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "Application.hpp"

static void glfwError(int id, const char* description)
{
    std::cout << "Glfw error: " << description << std::endl;
}

static bool update_vertex_buffer = true;

static void HandleEvents(GLFWwindow* window)
{
    glfwWaitEvents();

    if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT))
    {
        update_vertex_buffer = true;
        App::Layers::DoCurrentTool();
    }
}

int main(int argc, const char* argv[])
{
    if (!glfwInit())
        return -1;

    glfwSetErrorCallback(&glfwError);

    //glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    //glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    //glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    int window_width = 1280;
    int window_height = 700;

    /* Create a windowed mode window and its OpenGL context */
    GLFWwindow* window = glfwCreateWindow(window_width, window_height, "PixelCraft", nullptr, nullptr);

    if (!window)
    {
        std::cout << "Failed to create window\n";
        glfwTerminate();
        std::cin.get();
        return -1;
    }

    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);

    glfwMaximizeWindow(window);

    if (glewInit() != GLEW_OK)
    {
        std::cout << "Glew init error\n";
    }

    std::cout << "OpenGL version: " << glGetString(GL_VERSION) << '\n';

    GLCall(glEnable(GL_DEPTH_TEST));
    GLCall(glDepthFunc(GL_GREATER));

    GLCall(glEnable(GL_BLEND));
    GLCall(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));

    App::UI::ImGuiInit(window);

    glm::mat4 proj = glm::ortho(0.0f, (float)App::CANVAS_WIDTH, 0.0f, (float)App::CANVAS_HEIGHT);

    {  // Made this block so all the OpenGL objects would get destroyed before calling glfwTerminate.
        
        std::vector<float> vertices;

        Gla::FrameBuffer imgui_window_fb(window_width, window_height);
        Gla::FrameBuffer::BindToDefaultFB();

        Gla::Renderer renderer;

        Gla::Texture2D brush_tool_texture("assets/brush_tool.png", Gla::GLMinMagFilter::NEAREST, true);
        Gla::Texture2D eraser_tool_texture("assets/eraser_tool.png", Gla::GLMinMagFilter::NEAREST);
        Gla::Texture2D color_picker_tool_texture("assets/color_picker_tool.png", Gla::GLMinMagFilter::NEAREST);
        Gla::Texture2D bucket_tool_texture("assets/bucket_tool.png", Gla::GLMinMagFilter::NEAREST);
        
        Gla::Texture2D eye_opened_texture("assets/eye_opened.png", Gla::GLMinMagFilter::LINEAR);
        Gla::Texture2D eye_closed_texture("assets/eye_closed.png", Gla::GLMinMagFilter::LINEAR);
        Gla::Texture2D lock_locked_texture("assets/lock_locked.png", Gla::GLMinMagFilter::LINEAR);
        Gla::Texture2D lock_unlocked_texture("assets/lock_unlocked.png", Gla::GLMinMagFilter::LINEAR);

        App::UI::SetupToolTextures(
            brush_tool_texture.GetID(),
            eraser_tool_texture.GetID(),
            color_picker_tool_texture.GetID(),
            bucket_tool_texture.GetID()
        );

        App::UI::SetupLayerToolTextures(
            eye_opened_texture.GetID(),
            eye_closed_texture.GetID(),
            lock_locked_texture.GetID(),
            lock_unlocked_texture.GetID()
        );

        Gla::VertexArray va;
        Gla::VertexBuffer vb(nullptr, 0);
        Gla::VertexBufferLayout layout;
        layout.Push<float>(2);
        layout.Push<float>(4);
        va.AddBuffer(vb, layout);
        Gla::Shader shader("shader/VertShader.vert", "shader/FragShader.frag");
        shader.Bind();
        shader.SetUniformMat4f("u_ViewProjection", proj);
        Gla::Mesh mesh(va, shader);
        mesh.Bind();

        /* Loop until the user closes the window */
        while (!glfwWindowShouldClose(window))
        {
            App::UI::Update();

            App::UI::NewFrame();

            App::UI::RenderUI();
            App::UI::RenderDrawWindow(imgui_window_fb.GetTextureID(), "Draw");

            App::UI::RenderAndEndFrame();

            ImVec2 draw_window_dims = App::UI::GetDrawWinDimensions();

            float canvas_lenght = std::min(draw_window_dims.x, draw_window_dims.y);

            if (canvas_lenght > 1.0f)
            {
                imgui_window_fb.Bind();
                imgui_window_fb.Rescale((int)canvas_lenght, (int)canvas_lenght);
                mesh.Bind();

                if (update_vertex_buffer || App::UI::ShouldUpdateVertexBuffer())
                {
                    App::Layers::EmplaceVertices(vertices);
                    vb.UpdateSizeIfNeeded(vertices.size() * sizeof(float));
                    vb.UpdateData(vertices.data(), vertices.size() * sizeof(float));

                    update_vertex_buffer = false;
                }

                renderer.Clear();
                renderer.DrawArrays(GL_TRIANGLES, vertices.size() / 6);

                Gla::FrameBuffer::BindToDefaultFB();
            }

            /* Swap front and back buffers */
            glfwSwapBuffers(window);

            HandleEvents(window);
        }
    }

    // Cleanup
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}