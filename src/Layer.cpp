#include "Layer.hpp"

#include "Application.hpp"
#include "Definitions.hpp"
#include "Project.hpp"
#include "Tool.hpp"

#include <glm/geometric.hpp>

#include <algorithm>
#include <array>
#include <chrono>
#include <cmath>
#include <list>
#include <queue>
#include <ranges>
#include <vector>

namespace App
{
auto Color::operator=(const ImVec4& color) -> Color&
{
    r = static_cast<uint8_t>(color.x * 255);
    g = static_cast<uint8_t>(color.y * 255);
    b = static_cast<uint8_t>(color.z * 255);
    a = static_cast<uint8_t>(color.w * 255);
    return *this;
}

auto Color::operator==(const Color& other) const -> bool
{
    return other.r == r && other.g == g && other.b == b && other.a == a;
}

auto Color::operator==(const ImVec4& other) const -> bool
{
    constexpr float kTolerance = 0.0025F;

    return std::abs(static_cast<float>(r) / 0xff - other.x) <= kTolerance &&
           std::abs(static_cast<float>(g) / 0xff - other.y) <= kTolerance &&
           std::abs(static_cast<float>(b) / 0xff - other.z) <= kTolerance &&
           std::abs(static_cast<float>(a) / 0xff - other.w) <= kTolerance;
}

auto Color::BlendColor(Color color1, Color color2) -> Color
{
    Color result;

    float alpha1 = static_cast<float>(color1.a) / 255.0F;
    float alpha2 = static_cast<float>(color2.a) / 255.0F;
    float result_alpha = alpha1 + alpha2 * (1 - alpha1);

    result.r = static_cast<uint8_t>(
        (static_cast<float>(color1.r) * alpha1 +
         static_cast<float>(color2.r) * alpha2 * (1 - alpha1)) /
        result_alpha);
    result.g = static_cast<uint8_t>(
        (static_cast<float>(color1.g) * alpha1 +
         static_cast<float>(color2.g) * alpha2 * (1 - alpha1)) /
        result_alpha);
    result.b = static_cast<uint8_t>(
        (static_cast<float>(color1.b) * alpha1 +
         static_cast<float>(color2.b) * alpha2 * (1 - alpha1)) /
        result_alpha);
    result.a = static_cast<uint8_t>(result_alpha * 255);

    return result;
}

auto Color::FromImVec4(const ImVec4 color) -> Color
{
    return {static_cast<uint8_t>(color.x * 0xff),
            static_cast<uint8_t>(color.y * 0xff),
            static_cast<uint8_t>(color.z * 0xff),
            static_cast<uint8_t>(color.w * 0xff)};
}

Layer::Layer() noexcept
    : mCanvas(Project::CanvasHeight(),
              std::vector<Color>(Project::CanvasWidth())),
      mLayerName("Layer " + std::to_string(Layers::GetLayerCount() + 1))
{
}

void Layer::DoCurrentTool()
{
    // We also don't want to edit the canvas if it isn't visible
    if (mLocked || !mVisible) { return; }

    Vec2Int canvas;
    if (!CanvasCoordsFromCursorPos(canvas)) { return; }

    UI::SetVertexBuffUpdateToTrue();

    if (Tool::GetToolType() == kColorPicker)
    {
        auto displayed_canvas = Layers::GetDisplayedCanvas();
        const Color& picked_color = displayed_canvas[canvas.y][canvas.x];

        if (picked_color.a == 0) { return; }

        Tool::sCurrentColor->x = static_cast<float>(picked_color.r) / 0xff;
        Tool::sCurrentColor->y = static_cast<float>(picked_color.g) / 0xff;
        Tool::sCurrentColor->z = static_cast<float>(picked_color.b) / 0xff;

        return;
    }

    if (Tool::GetToolType() == kBucket)
    {
        Color clicked_color = mCanvas[canvas.y][canvas.x];
        Fill(canvas.x, canvas.y, clicked_color);
        return;
    }

    constexpr auto kMaxTimeInterval = std::chrono::milliseconds(200);
    static auto time_last_drawn = std::chrono::steady_clock::now();
    static auto position_last_drawn = canvas;

    if (Tool::GetBrushRadius() == 1)
    {
        if (Tool::GetToolType() == ToolType::kEraser)
        {
            mCanvas[canvas.y][canvas.x] = Color{0, 0, 0, 0};
        }
        else { mCanvas[canvas.y][canvas.x] = Tool::GetColorRef(); }
    }
    else { DrawCircle({canvas.x, canvas.y}, Tool::GetBrushRadius(), false); }

    if (glm::distance<2, float>(glm::vec2(canvas),
                                glm::vec2(position_last_drawn)) > 1 &&
        std::chrono::steady_clock::now() - time_last_drawn <= kMaxTimeInterval)
    {
        DrawLine(canvas, position_last_drawn, Tool::GetBrushRadius() * 2);
    }

    time_last_drawn = std::chrono::steady_clock::now();
    position_last_drawn = canvas;
}

void Layer::EmplaceVertices(std::vector<Vertex>& vertices,
                            bool use_color_alpha /*= false*/)
{
    if (!mVisible || mOpacity == 0) { return; }

    for (int i = 0; i < Project::CanvasHeight(); i++)
    {
        for (int j = 0; j < Project::CanvasWidth(); j++)
        {
            if (mCanvas[i][j].a == 0) { continue; }

            uint8_t alpha_val = 0;
            if (use_color_alpha) { alpha_val = mCanvas[i][j].a; }
            else { alpha_val = mOpacity; }

            Color color_used = mCanvas[i][j];
            color_used.a = alpha_val;

            /* first triangle */
            // upper left corner
            vertices.emplace_back(static_cast<float>(j), static_cast<float>(i),
                                  color_used);
            // upper right corner
            vertices.emplace_back(static_cast<float>(j) + 1,
                                  static_cast<float>(i), color_used);
            // bottom left corner
            vertices.emplace_back(static_cast<float>(j),
                                  static_cast<float>(i) + 1, color_used);
            /* second triangle */
            // upper right corner
            vertices.emplace_back(static_cast<float>(j) + 1,
                                  static_cast<float>(i), color_used);
            // bottom right corner
            vertices.emplace_back(static_cast<float>(j) + 1,
                                  static_cast<float>(i) + 1, color_used);
            // bottom left corner
            vertices.emplace_back(static_cast<float>(j),
                                  static_cast<float>(i) + 1, color_used);
        }
    }
}

void Layer::DrawCircle(Vec2Int center, int radius, bool only_outline,
                       Color delete_color /*= {0, 0, 0, 0}*/)
{
    if (radius < 1) { return; }

    Vec2Int dims = {Project::CanvasWidth(), Project::CanvasHeight()};

    Color draw_color = delete_color;

    if (Tool::GetToolType() != ToolType::kEraser)
    {
        draw_color = Tool::GetColorRef();
        draw_color.a = 0xff;
    }

    if (radius == 1)
    {
        mCanvas[center.y][center.x] = draw_color;
        return;
    }

    const int width = Project::CanvasWidth();
    const int height = Project::CanvasHeight();
    for (int xcrd = -radius; xcrd <= radius; xcrd++)
    {
        for (int ycrd = -radius; ycrd <= radius; ycrd++)
        {
            if (xcrd * xcrd + ycrd * ycrd < radius * radius)
            {
                int real_x = std::clamp(xcrd + center.x, 0, width - 1);
                int real_y = std::clamp(ycrd + center.y, 0, height - 1);
                mCanvas[real_y][real_x] = draw_color;
            }
        }
    }

    return;

    // Circle equation
    for (int x_coord = std::max(0, center.x - radius + 1);
         x_coord < std::min(Project::CanvasWidth(), center.x + radius);
         x_coord++)
    {
        int x_relative = x_coord - center.x;
        double y1_coord = std::sqrt(radius * radius - x_relative * x_relative);
        double y2_coord = -y1_coord;
        y1_coord += center.y;
        y2_coord += center.y;

        // If the number is round floor and ceil don't change anything
        if (y1_coord == static_cast<int>(y1_coord)) { y1_coord--; }
        if (y2_coord == static_cast<int>(y2_coord)) { y2_coord++; }

        int y1_floor = std::floor(y1_coord);
        int y2_ceil = std::ceil(y2_coord);

        if (y1_floor < 0) { y1_floor = 0; }
        else if (y1_floor >= Project::CanvasHeight())
        {
            y1_floor = Project::CanvasHeight() - 1;
        }

        if (y2_ceil < 0) { y2_ceil = 0; }
        else if (y2_ceil >= Project::CanvasHeight())
        {
            y2_ceil = Project::CanvasHeight() - 1;
        }

        mCanvas[y1_floor][x_coord] = draw_color;
        mCanvas[y2_ceil][x_coord] = draw_color;

        if (only_outline && x_coord != std::max(0, center.x - radius + 1) &&
            x_coord != std::min(Project::CanvasWidth(), center.x + radius) - 1)
        {
            continue;
        }

        for (int y_coord = static_cast<int>(y2_ceil) + 1; y_coord < y1_floor;
             y_coord++)
        {
            mCanvas[y_coord][x_coord] = draw_color;
        }
    }
}

void Layer::DrawLine(Vec2Int point_a, Vec2Int point_b, int thickness)
{
    if (thickness == 1)
    {
        DrawLine(point_a, point_b);
        return;
    }

    int radius = thickness / 2;
    int diff_x = std::abs(point_a.x - point_b.x);
    int diff_y = std::abs(point_a.y - point_b.y);
    int sign_x = (point_a.x < point_b.x) ? 1 : -1;
    int sign_y = (point_a.y < point_b.y) ? 1 : -1;
    int err = diff_x - diff_y;

    while (point_a != point_b)
    {
        int err2 = err;

        if (err2 > -diff_y)
        {
            err -= diff_y;
            point_a.x += sign_x;
        }

        if (err2 < diff_x)
        {
            err += diff_x;
            point_a.y += sign_y;
        }

        DrawCircle(point_a, radius, false);
    }
}

void Layer::DrawLine(Vec2Int point_a, Vec2Int point_b)
{
    int diff_x = std::abs(point_a.x - point_b.x);
    int diff_y = std::abs(point_a.y - point_b.y);
    int sign_x = (point_a.x < point_b.x) ? 1 : -1;
    int sign_y = (point_a.y < point_b.y) ? 1 : -1;
    int err = diff_x - diff_y;

    while (point_a != point_b)
    {
        int err2 = err;

        if (err2 > -diff_y)
        {
            err -= diff_y;
            point_a.x += sign_x;
        }

        if (err2 < diff_x)
        {
            err += diff_x;
            point_a.y += sign_y;
        }

        mCanvas[point_a.x][point_a.y] = Tool::GetColorRef();
    }
}

void Layer::Fill(int x_coord, int y_coord, Color clicked_color)
{
    if (x_coord < 0 || x_coord >= Project::CanvasWidth() || y_coord < 0 ||
        y_coord >= Project::CanvasHeight())
    {
        return;
    }

    auto& fill_color = Tool::GetColorRef();

    std::queue<std::pair<int, int>> pixel_queue;
    pixel_queue.emplace(y_coord, x_coord);

    while (!pixel_queue.empty())
    {
        auto& top = pixel_queue.front();
        const int row = top.first;
        const int col = top.second;

        if (mCanvas[row][col] == clicked_color &&
            mCanvas[row][col] != fill_color)
        {
            mCanvas[row][col] = fill_color;

            if (col + 1 < Project::CanvasWidth())
            {
                pixel_queue.emplace(row, col + 1);
            }

            if (col - 1 >= 0) { pixel_queue.emplace(row, col - 1); }

            if (row + 1 < Project::CanvasHeight())
            {
                pixel_queue.emplace(row + 1, col);
            }

            if (row - 1 >= 0) { pixel_queue.emplace(row - 1, col); }
        }

        pixel_queue.pop();
    }
}

// Returns false if the cursor isn't inside the canvas
auto Layer::CanvasCoordsFromCursorPos(Vec2Int& coords) -> bool
{
    double cursor_x = NAN;
    double cursor_y = NAN;
    // Cursor position relative to the Glfw window
    glfwGetCursorPos(UI::GetWindowPointer(), &cursor_x, &cursor_y);

    int window_x = 0;
    int window_y = 0;
    // Position of the window relative to the screen
    glfwGetWindowPos(UI::GetWindowPointer(), &window_x, &window_y);

    cursor_x += window_x; // Getting cursor position relative to the screen
    cursor_y += window_y;

    ImVec2 canvas_upperleft = UI::GetCanvasUpperleftCoords();
    ImVec2 canvas_bottomtright = UI::GetCanvasBottomRightCoords();

    if (cursor_x <= canvas_upperleft.x || cursor_x >= canvas_bottomtright.x ||
        cursor_y <= canvas_upperleft.y || cursor_y >= canvas_bottomtright.y)
    {
        return false;
    }

    coords.x = static_cast<int>((cursor_x - canvas_upperleft.x) /
                                ((canvas_bottomtright.x - canvas_upperleft.x) /
                                 static_cast<float>(Project::CanvasWidth())));
    coords.y = static_cast<int>((cursor_y - canvas_upperleft.y) /
                                ((canvas_bottomtright.y - canvas_upperleft.y) /
                                 static_cast<float>(Project::CanvasHeight())));

    return coords.x >= 0 && coords.x < Project::CanvasWidth() &&
           coords.y >= 0 && coords.y < Project::CanvasHeight();
}

auto Layers::GetCurrentLayer() -> Layer&
{
    MY_ASSERT(sCurrentLayerIndex >= 0 &&
              sCurrentLayerIndex < GetLayers().size());

    auto iter = GetLayers().begin();
    std::advance(iter, sCurrentLayerIndex);
    return *iter;
}

void Layers::DoCurrentTool()
{
    if (!UI::ShouldDoTool()) { return; }

    GetCurrentLayer().DoCurrentTool();
}

void Layers::AddLayer()
{
    GetLayers().emplace_back();
}

void Layers::MoveUp(std::size_t layer_index)
{
    // Can't move the top index up
    if (layer_index <= 0) { return; }

    auto it1 = GetLayers().begin();
    std::advance(it1, layer_index);
    auto it2 = GetLayers().begin();
    std::advance(it2, layer_index - 1);

    std::iter_swap(it1, it2);

    // The index of the current layer decreases by 1, since it goes up
    if (sCurrentLayerIndex == layer_index) { sCurrentLayerIndex--; }
    // If the current layer is the one moved down, the index increases by 1,
    // since it goes down
    else if (sCurrentLayerIndex == layer_index - 1) { sCurrentLayerIndex++; }
}

void Layers::MoveDown(std::size_t layer_index)
{
    // Can't move the bottom index down
    if (layer_index >= GetLayers().size() - 1) { return; }

    auto it1 = GetLayers().begin();
    std::advance(it1, layer_index);
    auto it2 = GetLayers().begin();
    std::advance(it2, layer_index + 1);

    std::iter_swap(it1, it2);

    // The index of the current layer increases by 1, since it goes down
    if (sCurrentLayerIndex == layer_index) { sCurrentLayerIndex++; }
    // If the current layer is the one moved up, the index decreases by 1, since
    // it goes up
    else if (sCurrentLayerIndex == layer_index + 1) { sCurrentLayerIndex--; }
}

void Layers::EmplaceVertices(std::vector<Vertex>& vertices)
{
    vertices.clear();

    constexpr std::array<Color, 2> kBgColors = {Color{131, 131, 131, 255},
                                                Color{201, 201, 201, 255}};

    /* Color{0.514F, 0.514F, 0.514F, 1.0F}, */
    /* Color{0.788F, 0.788F, 0.788F, 1.0F}}; */

    /* Background vertices */
    for (int i = 0; i < Project::CanvasHeight() + 6; i += 6)
    {
        for (int j = 0; j < Project::CanvasWidth() + 6; j += 6)
        {
            /* first triangle */
            // upper left corner
            vertices.emplace_back(static_cast<float>(j), static_cast<float>(i),
                                  kBgColors.at(((i + j) / 6) % 2));
            // upper right corner
            vertices.emplace_back(static_cast<float>(j) + 6,
                                  static_cast<float>(i),
                                  kBgColors.at(((i + j) / 6) % 2));
            // bottom left corner
            vertices.emplace_back(static_cast<float>(j),
                                  static_cast<float>(i) + 6,
                                  kBgColors.at(((i + j) / 6) % 2));
            /* second triangle */
            // upper right corner
            vertices.emplace_back(static_cast<float>(j) + 6,
                                  static_cast<float>(i),
                                  kBgColors.at(((i + j) / 6) % 2));
            // bottom right corner
            vertices.emplace_back(static_cast<float>(j) + 6,
                                  static_cast<float>(i) + 6,
                                  kBgColors.at(((i + j) / 6) % 2));
            // bottom left corner
            vertices.emplace_back(static_cast<float>(j),
                                  static_cast<float>(i) + 6,
                                  kBgColors.at(((i + j) / 6) % 2));
        }
    }

    for (auto& layer : std::ranges::reverse_view(GetLayers()))
    {
        layer.EmplaceVertices(vertices);
    }

    GetTempLayer().EmplaceVertices(vertices, true);
}

auto Layers::AtIndex(std::size_t index) -> Layer&
{
    MY_ASSERT(index >= 0 && index < GetLayers().size());

    auto iter = GetLayers().begin();
    std::advance(iter, index);

    return *iter;
}

void Layers::ResetDataToDefault()
{
    GetLayers().clear();
    sCurrentLayerIndex = 0;
}

void Layers::DrawToTempLayer()
{
    constexpr Color kNoColor = {0, 0, 0, 0};
    constexpr Color kDeleteColor = {255, 255, 255, 110};

    auto curr_tool = Tool::GetToolType();

    if (curr_tool != ToolType::kBrush && curr_tool != ToolType::kEraser)
    {
        return;
    }

    auto& tmp_layer = GetTempLayer();

    for (std::size_t i = 0;
         i < static_cast<std::size_t>(Project::CanvasHeight()); i++)
    {
        for (std::size_t j = 0;
             j < static_cast<std::size_t>(Project::CanvasWidth()); j++)
        {
            tmp_layer.mCanvas[i][j] = kNoColor;
        }
    }

    Vec2Int canvas_coords;
    if (!Layer::CanvasCoordsFromCursorPos(canvas_coords)) { return; }

    tmp_layer.DrawCircle(canvas_coords, Tool::GetBrushRadius(), false,
                         kDeleteColor);
}

auto Layers::GetDisplayedCanvas() -> CanvasData
{
    CanvasData displayed_canvas(Project::CanvasHeight(),
                                std::vector<Color>(Project::CanvasWidth()));

    for (auto& layer_traversed : std::ranges::reverse_view(GetLayers()))
    {
        for (int i = 0; i < Project::CanvasHeight(); i++)
        {
            for (int j = 0; j < Project::CanvasWidth(); j++)
            {
                Color dst_color = {
                    layer_traversed.mCanvas[i][j].r,
                    layer_traversed.mCanvas[i][j].g,
                    layer_traversed.mCanvas[i][j].b,
                    layer_traversed.mCanvas[i][j].a == 0
                        ? layer_traversed.mCanvas[i][j].a
                        : static_cast<uint8_t>(layer_traversed.mOpacity)};

                Color::BlendColor(displayed_canvas[i][j], dst_color);
            }
        }
    }

    return displayed_canvas;
}
} // namespace App
