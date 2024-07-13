#include "Layer.hpp"
#include "Application.hpp"
#include "Tool.hpp"
#include "Project.hpp"
#include "Definitions.hpp"

#include <iostream>
#include <vector>
#include <list>
#include <algorithm>
#include <queue>

// Normalize alpha from range [0, 255] to range [0.0, 1.0]
static float normalize_alpha(int alpha)
{
    return (float)alpha / 255.0f;
}

namespace App
{
    void Color::operator=(const ImVec4& color)
    {
        r = color.x;
        g = color.y;
        b = color.z;
        a = color.w;
    }

    bool Color::operator==(const Color& other) const
    {
        constexpr float tolerance = 0.0025f;

        if (std::abs(r - other.r) <= tolerance &&
            std::abs(g - other.g) <= tolerance &&
            std::abs(b - other.b) <= tolerance &&
            std::abs(a - other.a) <= tolerance)
        {
            return true;
        }

        return false;
    }

    bool Color::operator==(const ImVec4& other) const
    {
        constexpr float tolerance = 0.0025f;

        if (std::abs(r - other.x) <= tolerance &&
            std::abs(g - other.y) <= tolerance &&
            std::abs(b - other.z) <= tolerance &&
            std::abs(a - other.w) <= tolerance)
        {
            return true;
        }

        return false;
    }

    Color Color::BlendColor(Color src_color, Color dst_color)
    {
        if (src_color.a > 1.0f)  // Remember that alpha > 1.0f means there's no color. If there is no source color, just return the dest color
            return dst_color;

        Color result_color;

        // Calculate resulting alpha
        result_color.a = 1.0f - (1.0f - dst_color.a) * (1.0f - src_color.a);

        // Blend RGB channels
        result_color.r = (dst_color.r * dst_color.a / result_color.a) + (src_color.r * src_color.a * (1.0f - dst_color.a) / result_color.a);
        result_color.g = (dst_color.g * dst_color.a / result_color.a) + (src_color.g * src_color.a * (1.0f - dst_color.a) / result_color.a);
        result_color.b = (dst_color.b * dst_color.a / result_color.a) + (src_color.b * src_color.a * (1.0f - dst_color.a) / result_color.a);

        return result_color;
    }

    Color Color::FromImVec4(const ImVec4 color)
    {
        return { color.x, color.y, color.z, color.w };
    }

    Layer::Layer() :
        m_Canvas(Project::CanvasHeight(), std::vector<Color>(Project::CanvasWidth())),
        m_Visible(true),
        m_Locked(false),
        m_Opacity(255),
        m_LayerName("Layer " + std::to_string(Layers::GetLayerCount() + 1ull))
    {
    }

    void Layer::DoCurrentTool()
    {
        if (m_Locked || !m_Visible)  // We also don't want to edit the canvas if it isn't visible
            return;

        double cursor_x, cursor_y;
        glfwGetCursorPos(UI::GetWindowPointer(), &cursor_x, &cursor_y);  // Cursor position relative to the Glfw window
        int window_x, window_y;
        glfwGetWindowPos(UI::GetWindowPointer(), &window_x, &window_y);  // Position of the window relative to the screen
        cursor_x += window_x;  // Getting cursor position relative to the screen
        cursor_y += window_y;

        ImVec2 canvas_upperleft = UI::GetCanvasUpperleftCoords();
        ImVec2 canvas_bottomtright = UI::GetCanvasBottomRightCoords();

        if (cursor_x > canvas_upperleft.x && cursor_x < canvas_bottomtright.x &&
            cursor_y > canvas_upperleft.y && cursor_y < canvas_bottomtright.y)
        {
            int canvas_x = (int)((cursor_x - canvas_upperleft.x) / ((canvas_bottomtright.x - canvas_upperleft.x) / Project::CanvasWidth()));
            int canvas_y = (int)((cursor_y - canvas_upperleft.y) / ((canvas_bottomtright.y - canvas_upperleft.y) / Project::CanvasHeight()));

            if (canvas_x < 0 || canvas_x >= Project::CanvasWidth() ||
                canvas_y < 0 || canvas_y >= Project::CanvasHeight())
            {
                return;
            }

            UI::SetVertexBuffUpdateToTrue();

            if (Tool::GetToolType() == COLOR_PICKER)
            {
                auto displayed_canvas = Layers::GetDisplayedCanvas();
                const Color& picked_color = displayed_canvas[canvas_y][canvas_x];

                if (picked_color.a > 1.0f || picked_color.a <= 0.0025f)  // If alpha is greater than 1.0f nothing is drawn there and we don't want to pick a nonexistent color
                    return;

                Tool::s_CurrentColor->x = picked_color.r;
                Tool::s_CurrentColor->y = picked_color.g;
                Tool::s_CurrentColor->z = picked_color.b;

                return;
            }

            if (Tool::GetToolType() == BUCKET)
            {
                const Color& clicked_color = m_Canvas[canvas_y][canvas_x];

                Fill(canvas_x, canvas_y, clicked_color);
                return;
            }

            if (Tool::GetBrushRadius() == 1)
            {
                if (Tool::GetToolType() == ToolType::ERASER)
                {   // Alpha greater than 1.0f means nothing gets drawn
                    m_Canvas[canvas_y][canvas_x] = ImVec4{ 0.0f, 0.0f, 0.0f, 1.1f };
                }
                else
                {
                    m_Canvas[canvas_y][canvas_x] = Tool::GetColorRef();
                }
            }
            else
            {
                DrawCircle(canvas_x, canvas_y, Tool::GetBrushRadius(), false);
            }
        }
    }

    void Layer::EmplaceVertices(std::vector<float>& vertices)
    {
        if (!m_Visible)
            return;

        for (int i = 0; i < Project::CanvasHeight(); i++)
        {
            for (int j = 0; j < Project::CanvasWidth(); j++)
            {
                /* first triangle */

                float alpha_val = m_Canvas[i][j].a > 1.0f ? 0.0f : normalize_alpha(m_Opacity);

                // upper left corner
                vertices.push_back((float)j);
                vertices.push_back((float)i);
                vertices.push_back(m_Canvas[i][j].r);  // Color is a part of the vertex
                vertices.push_back(m_Canvas[i][j].g);
                vertices.push_back(m_Canvas[i][j].b);
                vertices.push_back(alpha_val);

                // upper right corner
                vertices.push_back((float)j + 1.0f);
                vertices.push_back((float)i);
                vertices.push_back(m_Canvas[i][j].r);
                vertices.push_back(m_Canvas[i][j].g);
                vertices.push_back(m_Canvas[i][j].b);
                vertices.push_back(alpha_val);

                // bottom left corner
                vertices.push_back((float)j);
                vertices.push_back((float)i + 1.0f);
                vertices.push_back(m_Canvas[i][j].r);
                vertices.push_back(m_Canvas[i][j].g);
                vertices.push_back(m_Canvas[i][j].b);
                vertices.push_back(alpha_val);


                /* second triangle */

                // upper right corner
                vertices.push_back((float)j + 1.0f);
                vertices.push_back((float)i);
                vertices.push_back(m_Canvas[i][j].r);
                vertices.push_back(m_Canvas[i][j].g);
                vertices.push_back(m_Canvas[i][j].b);
                vertices.push_back(alpha_val);

                // bottom right corner
                vertices.push_back((float)j + 1.0f);
                vertices.push_back((float)i + 1.0f);
                vertices.push_back(m_Canvas[i][j].r);
                vertices.push_back(m_Canvas[i][j].g);
                vertices.push_back(m_Canvas[i][j].b);
                vertices.push_back(alpha_val);

                // bottom left corner
                vertices.push_back((float)j);
                vertices.push_back((float)i + 1.0f);
                vertices.push_back(m_Canvas[i][j].r);
                vertices.push_back(m_Canvas[i][j].g);
                vertices.push_back(m_Canvas[i][j].b);
                vertices.push_back(alpha_val);
            }
        }
    }

    void Layer::DrawCircle(int center_x, int center_y, int radius, bool only_outline)
    {
        if (radius < 1)
        {
            return;
        }

        int x = radius;
        int y = 0;
        int err = 0;

        CanvasData tmp_canvas(Project::CanvasHeight(), std::vector<Color>(Project::CanvasWidth()));

        constexpr Color no_color = { 0.0f, 0.0f, 0.0f, 1.1f };
        constexpr Color delete_color = { 0.666f, 0.666f, 0.666f, 1.1f };  // Just a value that isn't equal to no_color
        Color draw_color = delete_color;  // default value

        if (Tool::GetToolType() != ToolType::ERASER)
        {
            draw_color = Tool::GetColorRef();
        }

        while (x >= y)
        {
            int x_coord = center_x + x, y_coord = center_y + y;

            // Checks if a coordinate is out of scope
            if (y_coord < 0)
                y_coord = 0;
            else if (y_coord >= Project::CanvasHeight())
                y_coord = Project::CanvasHeight() - 1;

            if (x_coord < 0)
                x_coord = 0;
            else if (x_coord >= Project::CanvasWidth())
                x_coord = Project::CanvasWidth() - 1;

            tmp_canvas[y_coord][x_coord] = draw_color;


            x_coord = center_x + y, y_coord = center_y + x;

            if (y_coord < 0)
                y_coord = 0;
            else if (y_coord >= Project::CanvasHeight())
                y_coord = Project::CanvasHeight() - 1;

            if (x_coord < 0)
                x_coord = 0;
            else if (x_coord >= Project::CanvasWidth())
                x_coord = Project::CanvasWidth() - 1;

            tmp_canvas[y_coord][x_coord] = draw_color;


            x_coord = center_x + x, y_coord = center_y - y;

            if (y_coord < 0)
                y_coord = 0;
            else if (y_coord >= Project::CanvasHeight())
                y_coord = Project::CanvasHeight() - 1;

            if (x_coord < 0)
                x_coord = 0;
            else if (x_coord >= Project::CanvasWidth())
                x_coord = Project::CanvasWidth() - 1;

            tmp_canvas[y_coord][x_coord] = draw_color;


            x_coord = center_x + y, y_coord = center_y - x;

            if (y_coord < 0)
                y_coord = 0;
            else if (y_coord >= Project::CanvasHeight())
                y_coord = Project::CanvasHeight() - 1;

            if (x_coord < 0)
                x_coord = 0;
            else if (x_coord >= Project::CanvasWidth())
                x_coord = Project::CanvasWidth() - 1;

            tmp_canvas[y_coord][x_coord] = draw_color;


            x_coord = center_x - x, y_coord = center_y + y;

            if (y_coord < 0)
                y_coord = 0;
            else if (y_coord >= Project::CanvasHeight())
                y_coord = Project::CanvasHeight() - 1;

            if (x_coord < 0)
                x_coord = 0;
            else if (x_coord >= Project::CanvasWidth())
                x_coord = Project::CanvasWidth() - 1;

            tmp_canvas[y_coord][x_coord] = draw_color;


            x_coord = center_x - y, y_coord = center_y + x;

            if (y_coord < 0)
                y_coord = 0;
            else if (y_coord >= Project::CanvasHeight())
                y_coord = Project::CanvasHeight() - 1;

            if (x_coord < 0)
                x_coord = 0;
            else if (x_coord >= Project::CanvasWidth())
                x_coord = Project::CanvasWidth() - 1;

            tmp_canvas[y_coord][x_coord] = draw_color;


            x_coord = center_x - x, y_coord = center_y - y;

            if (y_coord < 0)
                y_coord = 0;
            else if (y_coord >= Project::CanvasHeight())
                y_coord = Project::CanvasHeight() - 1;

            if (x_coord < 0)
                x_coord = 0;
            else if (x_coord >= Project::CanvasWidth())
                x_coord = Project::CanvasWidth() - 1;

            tmp_canvas[y_coord][x_coord] = draw_color;


            x_coord = center_x - y, y_coord = center_y - x;

            if (y_coord < 0)
                y_coord = 0;
            else if (y_coord >= Project::CanvasHeight())
                y_coord = Project::CanvasHeight() - 1;

            if (x_coord < 0)
                x_coord = 0;
            else if (x_coord >= Project::CanvasWidth())
                x_coord = Project::CanvasWidth() - 1;

            tmp_canvas[y_coord][x_coord] = draw_color;

            if (err <= 0)
            {
                y += 1;
                err += 2 * y + 1;
            }
            if (err > 0)
            {
                x -= 1;
                err -= 2 * x + 1;
            }
        }

        // These 4 if blocks are removing the tips at sides of the generated circle
        if (center_x + radius < Project::CanvasWidth())
        {
            tmp_canvas[center_y][center_x + radius] = no_color;
            tmp_canvas[center_y][center_x + radius - 1] = draw_color;
        }

        if (center_x - radius >= 0)
        {
            tmp_canvas[center_y][center_x - radius] = no_color;
            tmp_canvas[center_y][center_x - radius + 1] = draw_color;
        }

        if (center_y + radius < Project::CanvasHeight())
        {
            tmp_canvas[center_y + radius][center_x] = no_color;
            tmp_canvas[center_y + radius - 1][center_x] = draw_color;
        }

        if (center_y - radius >= 0)
        {
            tmp_canvas[center_y - radius][center_x] = no_color;
            tmp_canvas[center_y - radius + 1][center_x] = draw_color;
        }

        if (!only_outline)
        {
            for (int j = 0; j < Project::CanvasWidth(); j++)
            {
                for (int i = 0; i < Project::CanvasHeight(); i++)
                {
                    if (tmp_canvas[i][j] == no_color)
                        continue;

                    // Goes from the top pixel of each column and draws every pixel all the way to the bottom pixel or the bottom of the canvas
                    for (int k = i + 1; k < Project::CanvasHeight(); k++)
                    {
                        if (tmp_canvas[k][j] == no_color)
                            continue;

                        for (int l = i + 1; l < k; l++)
                            tmp_canvas[l][j] = draw_color;

                        break;
                    }
                }
            }
        }

        // Assigns the drawn pixels from the temporary canvas to the layer canvas
        for (int i = 0; i < Project::CanvasHeight(); i++)
        {
            for (int j = 0; j < Project::CanvasWidth(); j++)
            {
                if (tmp_canvas[i][j] != no_color)
                    m_Canvas[i][j] = tmp_canvas[i][j];
            }
        }
    }

    void Layer::Fill(int x, int y, Color clicked_color)
    {
        if (x < 0 || x >= Project::CanvasWidth() || y < 0 || y >= Project::CanvasHeight())
            return;
        
        auto& fill_color = Tool::GetColorRef();

        std::queue<std::pair<int, int>> pixel_queue;  // Queue of pixels waiting to be filled
        pixel_queue.emplace(y, x);

        while (!pixel_queue.empty())
        {
            auto& top = pixel_queue.front();
            const int row = top.first, col = top.second;

            if (m_Canvas[row][col] == clicked_color && m_Canvas[row][col] != fill_color)
            {
                m_Canvas[row][col] = fill_color;

                // Enqueue neighboring pixels
                if (col + 1 < Project::CanvasWidth())
                    pixel_queue.emplace(row, col + 1);

                if (col - 1 >= 0)
                    pixel_queue.emplace(row, col - 1);

                if (row + 1 < Project::CanvasHeight())
                    pixel_queue.emplace(row + 1, col);

                if (row - 1 >= 0)
                    pixel_queue.emplace(row - 1, col);
            }

            pixel_queue.pop();
        }
    }

    Layer& Layers::GetCurrentLayer()
    {
        MY_ASSERT(s_CurrentLayerIndex >= 0 && s_CurrentLayerIndex < s_Layers.size());

        auto it = s_Layers.begin();
        std::advance(it, s_CurrentLayerIndex);
        return *it;
    }

    void Layers::DoCurrentTool()
    {
        if (!UI::ShouldDoTool())
            return;

        GetCurrentLayer().DoCurrentTool();
    }

    void Layers::AddLayer()
    {
        s_Layers.emplace_back();
    }

    void Layers::MoveUp(int layer_index)
    {
        if (layer_index <= 0)  // Can't move the top index up
            return;

        auto it1 = s_Layers.begin();
        std::advance(it1, layer_index);
        auto it2 = s_Layers.begin();
        std::advance(it2, layer_index - 1);

        std::iter_swap(it1, it2);

        if (s_CurrentLayerIndex == layer_index)  // The index of the current layer decreases by 1, since it goes up
            s_CurrentLayerIndex--;
        else if (s_CurrentLayerIndex == layer_index - 1)  // If the current layer is the one moved down,
            s_CurrentLayerIndex++;                        // the index increases by 1, since it goes down
    }

    void Layers::MoveDown(int layer_index)
    {
        if (layer_index >= s_Layers.size() - 1)  // Can't move the bottom index down
            return;

        auto it1 = s_Layers.begin();
        std::advance(it1, layer_index);
        auto it2 = s_Layers.begin();
        std::advance(it2, layer_index + 1);

        std::iter_swap(it1, it2);

        if (s_CurrentLayerIndex == layer_index)  // The index of the current layer increases by 1, since it goes down
            s_CurrentLayerIndex++;
        else if (s_CurrentLayerIndex == layer_index + 1)  // If the current layer is the one moved up,
            s_CurrentLayerIndex--;                        // the index decreases by 1, since it goes up
    }
    
    void Layers::EmplaceVertices(std::vector<float>& vertices)
    {
        vertices.clear();

        constexpr Color bg_colors[2] = {
            { 0.514f, 0.514f, 0.514f, 1.0f },
            { 0.788f, 0.788f, 0.788f, 1.0f }
        };

        /* Background vertices */
        for (int i = 0; i < Project::CanvasHeight() + 6; i += 6)
        {
            for (int j = 0; j < Project::CanvasWidth() + 6; j += 6)
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

        for (auto rit = s_Layers.rbegin(); rit != s_Layers.rend(); rit++)
            rit->EmplaceVertices(vertices);
    }

    Layer& Layers::AtIndex(int index)
    {
        MY_ASSERT(index >= 0 && index < s_Layers.size());

        auto it = s_Layers.begin();
        std::advance(it, index);

        return *it;
    }

    void Layers::ResetDataToDefault()
    {
        s_Layers.clear();
        s_CurrentLayerIndex = 0;
    }

    const CanvasData Layers::GetDisplayedCanvas()
    {
        CanvasData displayed_canvas(Project::CanvasHeight(), std::vector<Color>(Project::CanvasWidth()));

        for (auto rit = s_Layers.rbegin(); rit != s_Layers.rend(); rit++)
        {
            Layer& layer_traversed = *rit;
            
            for (int i = 0; i < Project::CanvasHeight(); i++)
            {
                for (int j = 0; j < Project::CanvasWidth(); j++)
                {
                    Color dst_color = { layer_traversed.m_Canvas[i][j].r, layer_traversed.m_Canvas[i][j].g,
                    layer_traversed.m_Canvas[i][j].b, (float)layer_traversed.m_Opacity / 255.0f };

                    if (layer_traversed.m_Canvas[i][j].a > 1.0f)  // Alpha greater than 1.0f means there's no color
                        dst_color.a = 0.0f;

                    displayed_canvas[i][j] = Color::BlendColor(displayed_canvas[i][j], dst_color);
                }
            }
        }

        return displayed_canvas;
    }
}
