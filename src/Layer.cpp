#include "Layer.hpp"


#include "Application.hpp"
#include "Definitions.hpp"
#include "Project.hpp"
#include "Tool.hpp"

#include <algorithm>
#include <array>
#include <list>
#include <queue>
#include <ranges>
#include <vector>
#include <cmath>

// Normalize alpha from range [0, 255] to range [0.0, 1.0]
static auto NormalizeAlpha(int alpha) -> float
{
    return static_cast<float>(alpha) / 255.0F;
}

namespace App
{
auto Color::operator=(const ImVec4& color) -> Color&
{
    r = color.x;
    g = color.y;
    b = color.z;
    a = color.w;
    return *this;
}

auto Color::operator==(const Color& other) const -> bool
{
    constexpr float kTolerance = 0.0025F;

    return std::abs(r - other.r) <= kTolerance &&
           std::abs(g - other.g) <= kTolerance &&
           std::abs(b - other.b) <= kTolerance &&
           std::abs(a - other.a) <= kTolerance;
}

auto Color::operator==(const ImVec4& other) const -> bool
{
    constexpr float kTolerance = 0.0025F;

    return std::abs(r - other.x) <= kTolerance &&
           std::abs(g - other.y) <= kTolerance &&
           std::abs(b - other.z) <= kTolerance &&
           std::abs(a - other.w) <= kTolerance;
}

auto Color::BlendColor(Color src_color, Color dst_color) -> Color
{
    if (src_color.a > 1.0F)
    { // Remember that alpha > 1.0f means there's no color. If there is
        // no source color, just return the dest color
        return dst_color;
    }

    Color result_color;

    // Calculate resulting alpha
    result_color.a = 1.0F - (1.0F - dst_color.a) * (1.0F - src_color.a);

    // Blend RGB channels
    result_color.r =
        (dst_color.r * dst_color.a / result_color.a) +
        (src_color.r * src_color.a * (1.0F - dst_color.a) / result_color.a);
    result_color.g =
        (dst_color.g * dst_color.a / result_color.a) +
        (src_color.g * src_color.a * (1.0F - dst_color.a) / result_color.a);
    result_color.b =
        (dst_color.b * dst_color.a / result_color.a) +
        (src_color.b * src_color.a * (1.0F - dst_color.a) / result_color.a);

    return result_color;
}

auto Color::FromImVec4(const ImVec4 color) -> Color
{
    return {color.x, color.y, color.z, color.w};
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

        if (picked_color.a > 1.0F || picked_color.a <= 0.0025F)
        { // If alpha is greater than 1.0f nothing is drawn
            // there and we don't want to pick a nonexistent
            // color
            return;
        }

        Tool::sCurrentColor->x = picked_color.r;
        Tool::sCurrentColor->y = picked_color.g;
        Tool::sCurrentColor->z = picked_color.b;

        return;
    }

    if (Tool::GetToolType() == kBucket)
    {
        const Color& clicked_color = mCanvas[canvas.y][canvas.x];

        Fill(canvas.x, canvas.y, clicked_color);
        return;
    }

    if (Tool::GetBrushRadius() == 1)
    {
        if (Tool::GetToolType() == ToolType::kEraser)
        { // Alpha greater than 1.0f means nothing gets drawn
            mCanvas[canvas.y][canvas.x] = ImVec4{0.0F, 0.0F, 0.0F, 1.1F};
        }
        else { mCanvas[canvas.y][canvas.x] = Tool::GetColorRef(); }
    }
    else { DrawCircle({canvas.x, canvas.y}, Tool::GetBrushRadius(), false); }
}

void Layer::EmplaceVertices(std::vector<float>& vertices,
                            bool use_color_alpha /*= false*/)
{
    if (!mVisible) { return; }

    for (int i = 0; i < Project::CanvasHeight(); i++)
    {
        for (int j = 0; j < Project::CanvasWidth(); j++)
        {
            /* first triangle */

            float alpha_val = NAN;

            if (use_color_alpha) { alpha_val = mCanvas[i][j].a; }
            else
            {
                alpha_val =
                    mCanvas[i][j].a > 1.0F ? 0.0F : NormalizeAlpha(mOpacity);
            }

            // upper left corner
            vertices.push_back(static_cast<float>(j));
            vertices.push_back(static_cast<float>(i));
            vertices.push_back(
                mCanvas[i][j].r); // Color is a part of the vertex
            vertices.push_back(mCanvas[i][j].g);
            vertices.push_back(mCanvas[i][j].b);
            vertices.push_back(alpha_val);

            // upper right corner
            vertices.push_back(static_cast<float>(j) + 1.0F);
            vertices.push_back(static_cast<float>(i));
            vertices.push_back(mCanvas[i][j].r);
            vertices.push_back(mCanvas[i][j].g);
            vertices.push_back(mCanvas[i][j].b);
            vertices.push_back(alpha_val);

            // bottom left corner
            vertices.push_back(static_cast<float>(j));
            vertices.push_back(static_cast<float>(i) + 1.0F);
            vertices.push_back(mCanvas[i][j].r);
            vertices.push_back(mCanvas[i][j].g);
            vertices.push_back(mCanvas[i][j].b);
            vertices.push_back(alpha_val);

            /* second triangle */

            // upper right corner
            vertices.push_back(static_cast<float>(j) + 1.0F);
            vertices.push_back(static_cast<float>(i));
            vertices.push_back(mCanvas[i][j].r);
            vertices.push_back(mCanvas[i][j].g);
            vertices.push_back(mCanvas[i][j].b);
            vertices.push_back(alpha_val);

            // bottom right corner
            vertices.push_back(static_cast<float>(j) + 1.0F);
            vertices.push_back(static_cast<float>(i) + 1.0F);
            vertices.push_back(mCanvas[i][j].r);
            vertices.push_back(mCanvas[i][j].g);
            vertices.push_back(mCanvas[i][j].b);
            vertices.push_back(alpha_val);

            // bottom left corner
            vertices.push_back(static_cast<float>(j));
            vertices.push_back(static_cast<float>(i) + 1.0F);
            vertices.push_back(mCanvas[i][j].r);
            vertices.push_back(mCanvas[i][j].g);
            vertices.push_back(mCanvas[i][j].b);
            vertices.push_back(alpha_val);
        }
    }
}

void Layer::DrawCircle(Vec2Int center, int radius, bool only_outline,
                       Color delete_color /*= {0.0F, 0.0F, 0.0F, 1.0F}*/)
{
    if (radius < 1) { return; }

    CanvasData& canvas = mCanvas;
    Vec2Int dims = {Project::CanvasWidth(), Project::CanvasHeight()};

    /* constexpr Color kDeleteColor{0.0F, 0.0F, 0.0F, 1.1F}; */

    Color draw_color = delete_color;

    if (Tool::GetToolType() != ToolType::kEraser)
    {
        draw_color = Tool::GetColorRef();
    }

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

        canvas[y1_floor][x_coord] = draw_color;
        canvas[y2_ceil][x_coord] = draw_color;

        if (only_outline && x_coord != std::max(0, center.x - radius + 1) &&
            x_coord != std::min(Project::CanvasWidth(), center.x + radius) - 1)
        {
            continue;
        }

        for (int y_coord = static_cast<int>(y2_ceil) + 1; y_coord < y1_floor;
             y_coord++)
        {
            canvas[y_coord][x_coord] = draw_color;
        }
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

    std::queue<std::pair<int, int>>
        pixel_queue; // Queue of pixels waiting to be filled
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

            // Enqueue neighboring pixels
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

void Layers::EmplaceVertices(std::vector<float>& vertices)
{
    vertices.clear();

    constexpr std::array<Color, 2> kBgColors = {
        Color{0.514F, 0.514F, 0.514F, 1.0F},
        Color{0.788F, 0.788F, 0.788F, 1.0F}};

    /* Background vertices */
    for (int i = 0; i < Project::CanvasHeight() + 6; i += 6)
    {
        for (int j = 0; j < Project::CanvasWidth() + 6; j += 6)
        {
            /* first triangle */

            // upper left corner
            vertices.push_back(static_cast<float>(j));
            vertices.push_back(static_cast<float>(i));
            vertices.push_back(kBgColors.at(((i + j) / 6) % 2).r);
            vertices.push_back(kBgColors.at(((i + j) / 6) % 2).g);
            vertices.push_back(kBgColors.at(((i + j) / 6) % 2).b);
            vertices.push_back(kBgColors.at(((i + j) / 6) % 2).a);

            // upper right corner
            vertices.push_back(static_cast<float>(j) + 6.0F);
            vertices.push_back(static_cast<float>(i));
            vertices.push_back(kBgColors.at(((i + j) / 6) % 2).r);
            vertices.push_back(kBgColors.at(((i + j) / 6) % 2).g);
            vertices.push_back(kBgColors.at(((i + j) / 6) % 2).b);
            vertices.push_back(kBgColors.at(((i + j) / 6) % 2).a);

            // bottom left corner
            vertices.push_back(static_cast<float>(j));
            vertices.push_back(static_cast<float>(i) + 6.0F);
            vertices.push_back(kBgColors.at(((i + j) / 6) % 2).r);
            vertices.push_back(kBgColors.at(((i + j) / 6) % 2).g);
            vertices.push_back(kBgColors.at(((i + j) / 6) % 2).b);
            vertices.push_back(kBgColors.at(((i + j) / 6) % 2).a);

            /* second triangle */

            // upper right corner
            vertices.push_back(static_cast<float>(j) + 6.0F);
            vertices.push_back(static_cast<float>(i));
            vertices.push_back(kBgColors.at(((i + j) / 6) % 2).r);
            vertices.push_back(kBgColors.at(((i + j) / 6) % 2).g);
            vertices.push_back(kBgColors.at(((i + j) / 6) % 2).b);
            vertices.push_back(kBgColors.at(((i + j) / 6) % 2).a);

            // bottom right corner
            vertices.push_back(static_cast<float>(j) + 6.0F);
            vertices.push_back(static_cast<float>(i) + 6.0F);
            vertices.push_back(kBgColors.at(((i + j) / 6) % 2).r);
            vertices.push_back(kBgColors.at(((i + j) / 6) % 2).g);
            vertices.push_back(kBgColors.at(((i + j) / 6) % 2).b);
            vertices.push_back(kBgColors.at(((i + j) / 6) % 2).a);

            // bottom left corner
            vertices.push_back(static_cast<float>(j));
            vertices.push_back(static_cast<float>(i) + 6.0F);
            vertices.push_back(kBgColors.at(((i + j) / 6) % 2).r);
            vertices.push_back(kBgColors.at(((i + j) / 6) % 2).g);
            vertices.push_back(kBgColors.at(((i + j) / 6) % 2).b);
            vertices.push_back(kBgColors.at(((i + j) / 6) % 2).a);
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
    constexpr Color kNoColor = {0.0F, 0.0F, 0.0F, 0.0F};
    constexpr Color kDeleteColor = {1.0F, 1.0F, 1.0F, 0.4F};

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
                    static_cast<float>(layer_traversed.mOpacity) / 255.0F};

                // Alpha greater than 1.0f means there's no color
                if (layer_traversed.mCanvas[i][j].a > 1.0F)
                {
                    dst_color.a = 0.0F;
                }

                displayed_canvas[i][j] =
                    Color::BlendColor(displayed_canvas[i][j], dst_color);
            }
        }
    }

    return displayed_canvas;
}

auto Layers::GetLayers() -> std::list<Layer>&
{
    static std::list<Layer> layers;
    return layers;
}
} // namespace App
