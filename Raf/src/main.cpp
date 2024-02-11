#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#include <GL/glew.h>
#define GLFW_STATIC
#include <GLFW/glfw3.h>
#include <Gla/Gla.hpp>

#include <iostream>
#include <vector>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "Application.hpp"

static void glfwError(int id, const char* description)
{
    std::cout << "Glfw error: " << description << std::endl;
}

struct Color
{
    float r = 0.0f, g = 0.0f, b = 0.0f, a = 0.0f;

    void operator=(const ImVec4& color)
    {
        r = color.x;
        g = color.y;
        b = color.z;
        a = color.w;
    }
};

constexpr int CANVAS_HEIGHT = 32;
constexpr int CANVAS_WIDTH  = 32;
Color canvas[CANVAS_HEIGHT][CANVAS_WIDTH];

static void EmplaceVertices(std::vector<float>& vertices)
{
    vertices.clear();

    constexpr Color bg_colors[2] = {
        { 0.514f, 0.514f, 0.514f, 1.0f },
        { 0.788f, 0.788f, 0.788f, 1.0f }
    };

    /* Background vertices */
    for (int i = 0; i < CANVAS_HEIGHT + 6; i += 6)
    {
        for (int j = 0; j < CANVAS_WIDTH + 6; j += 6)
        {
            /* first triangle */

            // upper left corner
            vertices.push_back((float)j);
            vertices.push_back((float)i);
            vertices.push_back(bg_colors[((i + j) / 6) % 2].r);  // Color is a part of the vertex
            vertices.push_back(bg_colors[((i + j) / 6) % 2].g);
            vertices.push_back(bg_colors[((i + j) / 6) % 2].b);
            vertices.push_back(bg_colors[((i + j) / 6) % 2].a);

            // upper right corner
            vertices.push_back((float)j + 6.0f);
            vertices.push_back((float)i);
            vertices.push_back(bg_colors[((i + j) / 6) % 2].r);
            vertices.push_back(bg_colors[((i + j) / 6) % 2].g);
            vertices.push_back(bg_colors[((i + j) / 6) % 2].b);
            vertices.push_back(bg_colors[((i + j) / 6) % 2].a);

            // bottom left corner
            vertices.push_back((float)j);
            vertices.push_back((float)i + 6.0f);
            vertices.push_back(bg_colors[((i + j) / 6) % 2].r);
            vertices.push_back(bg_colors[((i + j) / 6) % 2].g);
            vertices.push_back(bg_colors[((i + j) / 6) % 2].b);
            vertices.push_back(bg_colors[((i + j) / 6) % 2].a);


            /* second triangle */

            // upper right corner
            vertices.push_back((float)j + 6.0f);
            vertices.push_back((float)i);
            vertices.push_back(bg_colors[((i + j) / 6) % 2].r);
            vertices.push_back(bg_colors[((i + j) / 6) % 2].g);
            vertices.push_back(bg_colors[((i + j) / 6) % 2].b);
            vertices.push_back(bg_colors[((i + j) / 6) % 2].a);

            // bottom right corner
            vertices.push_back((float)j + 6.0f);
            vertices.push_back((float)i + 6.0f);
            vertices.push_back(bg_colors[((i + j) / 6) % 2].r);
            vertices.push_back(bg_colors[((i + j) / 6) % 2].g);
            vertices.push_back(bg_colors[((i + j) / 6) % 2].b);
            vertices.push_back(bg_colors[((i + j) / 6) % 2].a);

            // bottom left corner
            vertices.push_back((float)j);
            vertices.push_back((float)i + 6.0f);
            vertices.push_back(bg_colors[((i + j) / 6) % 2].r);
            vertices.push_back(bg_colors[((i + j) / 6) % 2].g);
            vertices.push_back(bg_colors[((i + j) / 6) % 2].b);
            vertices.push_back(bg_colors[((i + j) / 6) % 2].a);
        }
    }

    /* Canvas vertices */
    for (int i = 0; i < CANVAS_HEIGHT; i++)
    {
        for (int j = 0; j < CANVAS_WIDTH; j++)
        {
            /* first triangle */

            // upper left corner
            vertices.push_back((float)j);
            vertices.push_back((float)i);
            vertices.push_back(canvas[i][j].r);  // Color is a part of the vertex
            vertices.push_back(canvas[i][j].g);
            vertices.push_back(canvas[i][j].b);
            vertices.push_back(canvas[i][j].a);
            
            // upper right corner
            vertices.push_back((float)j + 1.0f);
            vertices.push_back((float)i);
            vertices.push_back(canvas[i][j].r);
            vertices.push_back(canvas[i][j].g);
            vertices.push_back(canvas[i][j].b);
            vertices.push_back(canvas[i][j].a);
            
            // bottom left corner
            vertices.push_back((float)j);
            vertices.push_back((float)i + 1.0f);
            vertices.push_back(canvas[i][j].r);
            vertices.push_back(canvas[i][j].g);
            vertices.push_back(canvas[i][j].b);
            vertices.push_back(canvas[i][j].a);


            /* second triangle */

            // upper right corner
            vertices.push_back((float)j + 1.0f);
            vertices.push_back((float)i);
            vertices.push_back(canvas[i][j].r);
            vertices.push_back(canvas[i][j].g);
            vertices.push_back(canvas[i][j].b);
            vertices.push_back(canvas[i][j].a);

            // bottom right corner
            vertices.push_back((float)j + 1.0f);
            vertices.push_back((float)i + 1.0f);
            vertices.push_back(canvas[i][j].r);
            vertices.push_back(canvas[i][j].g);
            vertices.push_back(canvas[i][j].b);
            vertices.push_back(canvas[i][j].a);

            // bottom left corner
            vertices.push_back((float)j);
            vertices.push_back((float)i + 1.0f);
            vertices.push_back(canvas[i][j].r);
            vertices.push_back(canvas[i][j].g);
            vertices.push_back(canvas[i][j].b);
            vertices.push_back(canvas[i][j].a);
        }
    }
}

static void GenerateCircle(int centerX, int centerY, int radius, bool only_outline)
{
    if (radius < 1)
    {
        return;
    }

    int x = radius;
    int y = 0;
    int err = 0;

    Color tmp_canvas[CANVAS_HEIGHT][CANVAS_WIDTH];

    while (x >= y) {
        if (centerY + y >= 0 && centerY + y < CANVAS_HEIGHT && centerX + x >= 0 && centerX + x < CANVAS_WIDTH)
            tmp_canvas[centerY + y][centerX + x] = App::Brush::GetColorRef();

        if (centerY + x >= 0 && centerY + x < CANVAS_HEIGHT && centerX + y >= 0 && centerX + y < CANVAS_WIDTH)
            tmp_canvas[centerY + x][centerX + y] = App::Brush::GetColorRef();

        if (centerY - y >= 0 && centerY - y < CANVAS_HEIGHT && centerX + x >= 0 && centerX + x < CANVAS_WIDTH)
            tmp_canvas[centerY - y][centerX + x] = App::Brush::GetColorRef();
        
        if (centerY - x >= 0 && centerY - x < CANVAS_HEIGHT && centerX + y >= 0 && centerX + y < CANVAS_WIDTH)
            tmp_canvas[centerY - x][centerX + y] = App::Brush::GetColorRef();

        if (centerY + y >= 0 && centerY + y < CANVAS_HEIGHT && centerX - x >= 0 && centerX - x < CANVAS_WIDTH)
            tmp_canvas[centerY + y][centerX - x] = App::Brush::GetColorRef();

        if (centerY + x >= 0 && centerY + x < CANVAS_HEIGHT && centerX - y >= 0 && centerX - y < CANVAS_WIDTH)
            tmp_canvas[centerY + x][centerX - y] = App::Brush::GetColorRef();

        if (centerY - y >= 0 && centerY - y < CANVAS_HEIGHT && centerX - x >= 0 && centerX - x < CANVAS_WIDTH)
            tmp_canvas[centerY - y][centerX - x] = App::Brush::GetColorRef();

        if (centerY - x >= 0 && centerY - x < CANVAS_HEIGHT && centerX - y >= 0 && centerX - y < CANVAS_WIDTH)
            tmp_canvas[centerY - x][centerX - y] = App::Brush::GetColorRef();

        if (err <= 0) {
            y += 1;
            err += 2 * y + 1;
        }
        if (err > 0) {
            x -= 1;
            err -= 2 * x + 1;
        }
    }

    if (!only_outline)
    {
        for (int j = 0; j < CANVAS_WIDTH; j++)
        {
            for (int i = 0; i < CANVAS_HEIGHT; i++)
            {
                if (tmp_canvas[i][j].a == 0.0f)
                    continue;

                for (int k = i + 1; k < CANVAS_HEIGHT; k++)
                {
                    if (tmp_canvas[k][j].a == 0.0f)
                        continue;

                    for (int l = i + 1; l < k; l++)
                        tmp_canvas[l][j] = App::Brush::GetColorRef();

                    break;
                }
            }
        }
    }

    for (int i = 0; i < CANVAS_HEIGHT; i++)
    {
        for (int j = 0; j < CANVAS_WIDTH; j++)
        {
            if (tmp_canvas[i][j].a != 0.0f)
                canvas[i][j] = tmp_canvas[i][j];
        }
    }
}

static void HandleEvents(GLFWwindow* window)
{
#pragma message(": Try using glfwPollEvents in case there is a bug you think is realted to this")
    glfwWaitEvents();

    if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT))
    {
        double cursor_x, cursor_y;
        glfwGetCursorPos(window, &cursor_x, &cursor_y);

        ImVec2 canvas_upperleft     = App::UI::GetCanvasUpperleftCoords();
        ImVec2 canvas_bottomtright  = App::UI::GetCanvasBottomRightCoords();

        cursor_y += ((canvas_bottomtright.y - canvas_upperleft.y) / CANVAS_HEIGHT);

        if (cursor_x > canvas_upperleft.x && cursor_x < canvas_bottomtright.x &&
            cursor_y > canvas_upperleft.y && cursor_y < canvas_bottomtright.y)
        {
            int canvas_x = (cursor_x - canvas_upperleft.x) / ((canvas_bottomtright.x - canvas_upperleft.x) / CANVAS_WIDTH);
            int canvas_y = (cursor_y - canvas_upperleft.y) / ((canvas_bottomtright.y - canvas_upperleft.y) / CANVAS_HEIGHT);

            if (canvas_x < 0 || canvas_x >= CANVAS_WIDTH ||
                canvas_y < 0 || canvas_y >= CANVAS_HEIGHT)
            {
                LOG("Out of range");
                return;
            }

            if (App::Brush::GetBrushRadius() == 1)
            {
                canvas[canvas_y][canvas_x].r = App::Brush::GetColorRef().x;
                canvas[canvas_y][canvas_x].g = App::Brush::GetColorRef().y;
                canvas[canvas_y][canvas_x].b = App::Brush::GetColorRef().z;
                canvas[canvas_y][canvas_x].a = App::Brush::GetColorRef().w;
            }
            else
            {
                GenerateCircle(canvas_x, canvas_y, App::Brush::GetBrushRadius(), false);
            }
        }
    }
}

int main(int argc, const char* argv[])
{
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
    GLFWwindow* window = glfwCreateWindow(window_width, window_height, "PixelCraft", NULL, NULL);

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

    App::UI::ImGuiInit(window);

    glm::mat4 proj = glm::ortho(0.0f, (float)CANVAS_WIDTH, 0.0f, (float)CANVAS_HEIGHT);

    {  // Made this block so all the OpenGL objects would get destroyed before calling glfwTerminate.
        
        std::vector<float> vertices;

        Gla::FrameBuffer imgui_window_fb(window_width, window_height);
        Gla::FrameBuffer::BindToDefaultFB();

        Gla::Renderer renderer;

        //constexpr int VB_SIZE = CANVAS_WIDTH * CANVAS_HEIGHT * (2 + 4) * 6 * sizeof(float);

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

                // TODO: should optimize vertex updating later by putting it out of the main loop
                EmplaceVertices(vertices);
                //GLAssert(vertices.size() * sizeof(float) == VB_SIZE);
                vb.UpdateSizeIfShould((unsigned int)(vertices.size() * sizeof(float)));
                vb.UpdateData(vertices.data(), vertices.size() * sizeof(float));

                renderer.Clear();
                renderer.DrawArrays(GL_TRIANGLES, vertices.size() / 6);
                Gla::FrameBuffer::BindToDefaultFB();
            }

            /* Swap front and back buffers */
            glfwSwapBuffers(window);

            /* Poll for and process events */
            //glfwPollEvents();
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