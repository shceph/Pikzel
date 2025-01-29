#include "Layer.hpp"

#include "Application.hpp"
#include "Camera.hpp"
#include "Events.hpp"
#include "Project.hpp"
#include "Tool.hpp"
#include "VertexBufferControl.hpp"

#include "GLFW/glfw3.h"
#include <glm/geometric.hpp>

#include <algorithm>
#include <array>
#include <chrono>
#include <cmath>
#include <list>
#include <queue>
#include <ranges>
#include <vector>

namespace Pikzel
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
    ImVec4 col1 = {
        static_cast<float>(color1.r) / 255,
        static_cast<float>(color1.g) / 255,
        static_cast<float>(color1.b) / 255,
        static_cast<float>(color1.a) / 255,
    };

    ImVec4 col2 = {
        static_cast<float>(color2.r) / 255,
        static_cast<float>(color2.g) / 255,
        static_cast<float>(color2.b) / 255,
        static_cast<float>(color2.a) / 255,
    };

    float alpha1 = col1.w;
    float alpha2 = col2.w;
    float out_alpha = alpha1 + alpha2 * (1.0F - alpha1);

    if (out_alpha == 0) { return {0, 0, 0, 0}; }

    float out_r =
        (col1.x * alpha1 + col2.x * alpha2 * (1.0F - alpha1)) / out_alpha;
    float out_g =
        (col1.y * alpha1 + col2.y * alpha2 * (1.0F - alpha1)) / out_alpha;
    float out_b =
        (col1.z * alpha1 + col2.z * alpha2 * (1.0F - alpha1)) / out_alpha;

    return Color::FromImVec4({out_r, out_g, out_b, out_alpha});
}

auto Color::FromImVec4(const ImVec4 color) -> Color
{
    return {static_cast<uint8_t>(color.x * 0xff),
            static_cast<uint8_t>(color.y * 0xff),
            static_cast<uint8_t>(color.z * 0xff),
            static_cast<uint8_t>(color.w * 0xff)};
}

Layer::Layer(bool is_canvas_layer /*= true*/) noexcept
    : mCanvas(Project::CanvasHeight(),
              std::vector<Color>(Project::CanvasWidth())),
      mIsCanvasLayer(is_canvas_layer),
      mLayerName("Layer " + std::to_string(sConstructCounter))
{
    if (mIsCanvasLayer) { sConstructCounter++; }
}

void Layer::DoCurrentTool()
{
    if (mLocked || !mVisible) { return; }

    switch (Tool::GetToolType())
    {
    case kBrush:
    case kEraser:
        HandleBrushAndEraser();
        break;
    case kColorPicker:
        HandleColorPicker();
        break;
    case kBucket:
        HandleBucket();
        break;
    case kRectShape:
        HandleRectShape();
        break;
    case kToolCount:
        assert(false);
    }
}

void Layer::EmplaceVertices(std::vector<Vertex>& vertices,
                            bool use_color_alpha /*= false*/) const
{
    if (!mVisible || mOpacity == 0) { return; }

    for (int i = 0; i < Project::CanvasHeight(); i++)
    {
        for (int j = 0; j < Project::CanvasWidth(); j++)
        {
            /* if (mCanvas[i][j].a == 0) { continue; } */

            uint8_t alpha_val = 0;
            if (use_color_alpha) { alpha_val = mCanvas[i][j].a; }
            else { alpha_val = mOpacity; }

            Color color_used = mCanvas[i][j];

            if (color_used.a != 0) { color_used.a = alpha_val; }

            // first triangle
            // upper left corner
            vertices.emplace_back(static_cast<float>(j), static_cast<float>(i),
                                  color_used);
            // upper right corner
            vertices.emplace_back(static_cast<float>(j) + 1,
                                  static_cast<float>(i), color_used);
            // bottom left corner
            vertices.emplace_back(static_cast<float>(j),
                                  static_cast<float>(i) + 1, color_used);
            // second triangle
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

void Layer::Update()
{
}

void Layer::HandleBrushAndEraser()
{
    if (!Events::IsMouseButtonPressed(Events::kButtonLeft)) { return; }
    auto canv_coord = CanvasCoordsFromCursorPos();
    if (!canv_coord.has_value()) { return; }

    constexpr auto kMaxDelay = std::chrono::milliseconds(100);
    static auto time_last_drawn = std::chrono::steady_clock::now();
    static auto position_last_drawn = canv_coord.value();

    if (Tool::GetBrushRadius() == 1)
    {
        if (Tool::GetToolType() == ToolType::kEraser)
        {
            DrawPixel(canv_coord.value(), {0, 0, 0, 0});
        }
        else { DrawPixel(canv_coord.value()); }
    }
    else
    {
        DrawCircle({canv_coord->x, canv_coord->y}, Tool::GetBrushRadius(),
                   true);
    }

    if (glm::distance<2, float>(glm::vec2(canv_coord.value()),
                                glm::vec2(position_last_drawn)) > 1 &&
        std::chrono::steady_clock::now() - time_last_drawn <= kMaxDelay)
    {
        DrawLine(canv_coord.value(), position_last_drawn,
                 Tool::GetBrushRadius() * 2);
    }
    else { MarkHistoryForUpdate(); }

    time_last_drawn = std::chrono::steady_clock::now();
    position_last_drawn = canv_coord.value();
}

void Layer::HandleColorPicker()
{
    if (!Events::IsMouseButtonPressed(Events::kButtonLeft)) { return; }
    auto canv_coord = CanvasCoordsFromCursorPos();
    if (!canv_coord.has_value()) { return; }

    auto displayed_canvas = Layers::GetDisplayedCanvas();
    const Color picked_color = displayed_canvas[canv_coord->y][canv_coord->x];

    if (picked_color.a == 0) { return; }

    Tool::sCurrentColor->x = static_cast<float>(picked_color.r) / 0xff;
    Tool::sCurrentColor->y = static_cast<float>(picked_color.g) / 0xff;
    Tool::sCurrentColor->z = static_cast<float>(picked_color.b) / 0xff;
}

void Layer::HandleBucket()
{
    if (!Events::IsMouseButtonPressed(Events::kButtonLeft)) { return; }
    auto canv_coord = CanvasCoordsFromCursorPos();
    if (!canv_coord.has_value()) { return; }

    Color clicked_color = GetPixel(canv_coord.value());
    Fill(canv_coord->x, canv_coord->y, clicked_color);
    MarkHistoryForUpdate();
}

void Layer::HandleRectShape()
{
    auto canv_coord = CanvasCoordsFromCursorPos();
    if (!canv_coord.has_value()) { return; }
    bool left_button_pressed =
        Events::IsMouseButtonPressed(Events::kButtonLeft);

    /* static bool shape_began = false; */
    /* static Vec2Int shape_begin_coords{0, 0}; */

    if (!mHandleRectShapeData.shape_began)
    {
        if (left_button_pressed)
        {
            mHandleRectShapeData.shape_begin_coords = canv_coord.value();
            mHandleRectShapeData.shape_began = true;
        }
        return;
    }

    // Use left shift to force drawing a square
    if (Events::IsKeyboardKeyPressed(GLFW_KEY_LEFT_SHIFT))
    {
        int diff_x = mHandleRectShapeData.shape_begin_coords.x - canv_coord->x;
        int diff_y = mHandleRectShapeData.shape_begin_coords.y - canv_coord->y;

        if (std::abs(diff_x) < std::abs(diff_y))
        {
            canv_coord->y = mHandleRectShapeData.shape_begin_coords.y - diff_x;
        }
        else
        {
            canv_coord->x = mHandleRectShapeData.shape_begin_coords.x - diff_y;
        }
    }

    if (left_button_pressed)
    {
        if (IsPreviewLayer())
        {
            DrawRect(mHandleRectShapeData.shape_begin_coords,
                     canv_coord.value(), true);
        }

        return;
    }

    if (mIsCanvasLayer)
    {
        DrawRect(mHandleRectShapeData.shape_begin_coords, canv_coord.value(),
                 true);
    }
    else { Clear(); }

    mHandleRectShapeData.shape_began = false;
    MarkHistoryForUpdate();
}

void Layer::DrawPixel(Vec2Int coords,
                      Color color /*= Color::FromImVec4(Tool::GetColor())*/)
{
    std::unique_lock<std::mutex> lock{sMutex};
    mCanvas[coords.y][coords.x] = color;
    lock.unlock();

    if (mIsCanvasLayer)
    {
        VertexBufferControl::PushDirtyPixel(coords);
        GetDirtyPixels().push_back(coords);
    }
    /* VertexBufferControl::UpdatePixel(coords); */
}

void Layer::DrawCircle(Vec2Int center, int radius, bool fill,
                       Color delete_color /*= {0, 0, 0, 0}*/)
{
    if (radius < 1) { return; }

    Color draw_color = delete_color;

    if (Tool::GetToolType() != ToolType::kEraser)
    {
        draw_color = Tool::GetColor();
        draw_color.a = 0xff;
    }

    if (radius == 1)
    {
        DrawPixel(center, draw_color);
        return;
    }

    Vec2Int dims = {Project::CanvasWidth(), Project::CanvasHeight()};

    if (fill)
    {
        for (int xcrd = -radius; xcrd <= radius; xcrd++)
        {
            for (int ycrd = -radius; ycrd <= radius; ycrd++)
            {
                if (xcrd * xcrd + ycrd * ycrd < radius * radius)
                {
                    int real_x = std::clamp(xcrd + center.x, 0, dims.x - 1);
                    int real_y = std::clamp(ycrd + center.y, 0, dims.y - 1);
                    DrawPixel({real_x, real_y}, draw_color);
                }
            }
        }

        return;
    }

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

        /* mCanvas[y1_floor][x_coord] = draw_color; */
        /* mCanvas[y2_ceil][x_coord] = draw_color; */
        DrawPixel({x_coord, y1_floor}, draw_color);
        DrawPixel({x_coord, y2_ceil}, draw_color);
    }
}

void Layer::Clear()
{
    for (int i = 0; i < Project::CanvasHeight(); i++)
    {
        for (int j = 0; j < Project::CanvasWidth(); j++)
        {
            DrawPixel({j, i}, {0, 0, 0, 0});
        }
    }
}

void Layer::MarkHistoryForUpdate() const
{
    if (mIsCanvasLayer) { Layers::MarkHistoryForUpdate(); }
}

void Layer::DrawRect(Vec2Int upper_left, Vec2Int bottom_right, bool /*fill*/)
{
    std::size_t max_x = std::max(upper_left.x, bottom_right.x);
    std::size_t min_x = std::min(upper_left.x, bottom_right.x);
    std::size_t max_y = std::max(upper_left.y, bottom_right.y);
    std::size_t min_y = std::min(upper_left.y, bottom_right.y);

    for (auto i = min_y; i <= max_y; i++)
    {
        for (auto j = min_x; j <= max_x; j++)
        {
            // mCanvas[i][j] = Tool::GetColor();
            DrawPixel({j, i});
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

        DrawCircle(point_a, radius, true);
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

        // mCanvas[point_a.x][point_a.y] = Tool::GetColorRef();
        DrawPixel(point_a);
    }
}

void Layer::Fill(int x_coord, int y_coord, Color clicked_color)
{
    if (x_coord < 0 || x_coord >= Project::CanvasWidth() || y_coord < 0 ||
        y_coord >= Project::CanvasHeight())
    {
        return;
    }

    auto fill_color = Tool::GetColor();

    std::queue<std::pair<int, int>> pixel_queue;
    pixel_queue.emplace(y_coord, x_coord);

    while (!pixel_queue.empty())
    {
        auto& top = pixel_queue.front();
        const int row = top.first;
        const int col = top.second;
        Color pixel = GetPixel({col, row});

        if (pixel == clicked_color && pixel != fill_color)
        {
            DrawPixel({col, row}, Color::FromImVec4(fill_color));

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

auto Layer::CanvasCoordsFromCursorPos() -> std::optional<Vec2Int>
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
        return std::nullopt;
    }

    glm::vec2 cursor_draw_win_relative{cursor_x - canvas_upperleft.x,
                                       cursor_y - canvas_upperleft.y};
    glm::vec2 canvas_on_screen_dims{canvas_bottomtright.x - canvas_upperleft.x,
                                    canvas_bottomtright.y - canvas_upperleft.y};
    glm::vec2 canvas_dims{Project::GetCanvasDims()};

    Vec2Int coords =
        cursor_draw_win_relative / (canvas_on_screen_dims / canvas_dims);

    double zoom_val = Camera::GetZoomValue();
    if (zoom_val != 0)
    {
        double inv_zoom = 1 - zoom_val;
        int new_width = static_cast<int>(Project::CanvasWidth() * inv_zoom);
        int new_height = static_cast<int>(Project::CanvasHeight() * inv_zoom);
        coords.x = static_cast<int>(coords.x * inv_zoom);
        coords.y = static_cast<int>(coords.y * inv_zoom);
        coords.x += (Project::CanvasWidth() - new_width) / 2;
        coords.y += (Project::CanvasHeight() - new_height) / 2;
    }

    coords += Camera::GetCenterAsVec2Int() - Project::GetCanvasDims() / 2;

    if (coords.x < 0 || coords.x >= Project::CanvasWidth() || coords.y < 0 ||
        coords.y >= Project::CanvasHeight())
    {
        return std::nullopt;
    }

    return std::make_optional(coords);
}

auto Layer::ClampToCanvasDims(Vec2Int val_to_clamp) -> Vec2Int
{
    return glm::clamp(val_to_clamp, {0, 0}, Project::GetCanvasDims());
}

// Should call this func before VertexBufferControl::Update, since it needs
// dirty pixels
void Layer::UpdateStatic()
{
    sShouldUpdateWholeVBO = false;
}

auto Layers::GetCurrentLayer() -> Layer&
{
    assert(sCurrentLayerIndex >= 0 && sCurrentLayerIndex < GetLayers().size());

    auto iter = GetLayers().begin();
    std::advance(iter, sCurrentLayerIndex);
    return *iter;
}

void Layers::DoCurrentTool()
{
    GetCurrentLayer().DoCurrentTool();
}

void Layers::AddLayer()
{
    GetLayers().emplace_back();
}

void Layers::MoveUp(std::size_t layer_index)
{
    if (layer_index == 0) { return; }

    auto it1 = GetLayers().begin();
    std::advance(it1, layer_index);
    auto it2 = GetLayers().begin();
    std::advance(it2, layer_index - 1);
    std::iter_swap(it1, it2);

    if (sCurrentLayerIndex == layer_index) { sCurrentLayerIndex--; }
    else if (sCurrentLayerIndex == layer_index - 1) { sCurrentLayerIndex++; }
}

void Layers::MoveDown(std::size_t layer_index)
{
    if (layer_index >= GetLayers().size() - 1) { return; }

    auto it1 = GetLayers().begin();
    std::advance(it1, layer_index);
    auto it2 = GetLayers().begin();
    std::advance(it2, layer_index + 1);
    std::iter_swap(it1, it2);

    if (sCurrentLayerIndex == layer_index) { sCurrentLayerIndex++; }
    else if (sCurrentLayerIndex == layer_index + 1) { sCurrentLayerIndex--; }
}

void Layers::EmplaceVertices(std::vector<Vertex>& vertices)
{
    assert(Project::IsOpened());
    vertices.clear();

    for (auto& layer : GetLayers())
    {
        layer.EmplaceVertices(vertices);
    }
}

void Layers::EmplaceBckgVertices(std::vector<Vertex>& vertices)
{
    assert(Project::IsOpened());

    constexpr std::array<Color, 2> kBgColors = {Color{131, 131, 131, 255},
                                                Color{201, 201, 201, 255}};

    for (int i = 0; i < Project::CanvasHeight(); i += 6)
    {
        for (int j = 0; j < Project::CanvasWidth(); j += 6)
        {
            auto x_coord = static_cast<float>(j);
            auto y_coord = static_cast<float>(i);
            glm::vec2 dims = Project::GetCanvasDims();

            // upper left corner
            vertices.emplace_back(x_coord, y_coord,
                                  kBgColors.at(((i + j) / 6) % 2));
            // upper right corner
            vertices.emplace_back(std::clamp(x_coord + 6, 0.0F, dims.x),
                                  y_coord, kBgColors.at(((i + j) / 6) % 2));
            // bottom left corner
            vertices.emplace_back(x_coord,
                                  std::clamp(y_coord + 6, 0.0F, dims.y),
                                  kBgColors.at(((i + j) / 6) % 2));
            /* second triangle */
            // upper right corner
            vertices.emplace_back(std::clamp(x_coord + 6, 0.0F, dims.x),
                                  y_coord, kBgColors.at(((i + j) / 6) % 2));
            // bottom right corner
            vertices.emplace_back(std::clamp(x_coord + 6, 0.0F, dims.x),
                                  std::clamp(y_coord + 6, 0.0F, dims.y),
                                  kBgColors.at(((i + j) / 6) % 2));
            // bottom left corner
            vertices.emplace_back(x_coord,
                                  std::clamp(y_coord + 6, 0.0F, dims.y),
                                  kBgColors.at(((i + j) / 6) % 2));
        }
    }
}

auto Layers::AtIndex(std::size_t index) -> Layer&
{
    assert(index >= 0 && index < GetLayers().size());

    auto iter = GetLayers().begin();
    std::advance(iter, index);

    return *iter;
}

void Layers::ResetDataToDefault()
{
    GetLayers().clear();
    sCurrentLayerIndex = 0;
}

/*void Layers::DrawToTempLayer()
{
    constexpr Color kNoColor = {0, 0, 0, 0};
    constexpr Color kDeleteColor = {255, 255, 255, 110};

    auto& tmp_layer = GetTempLayer();

    for (std::size_t i = 0;
         i < static_cast<std::size_t>(Project::CanvasHeight()); i++)
    {
        for (std::size_t j = 0;
             j < static_cast<std::size_t>(Project::CanvasWidth()); j++)
        {
            // tmp_layer.mCanvas[i][j] = kNoColor;
            tmp_layer.DrawPixel({j, i}, kNoColor);
        }
    }

    auto curr_tool = Tool::GetToolType();

    if (curr_tool != ToolType::kBrush && curr_tool != ToolType::kEraser)
    {
        return;
    }

    std::optional<Vec2Int> canvas_coords = Layer::CanvasCoordsFromCursorPos();
    if (!canvas_coords.has_value()) { return; }

    tmp_layer.DrawCircle(canvas_coords.value(), Tool::GetBrushRadius(), true,
                         kDeleteColor);
}*/

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
                Color pixel = layer_traversed.GetPixel({j, i});

                Color dst_color = {
                    pixel.r, pixel.g, pixel.b,
                    pixel.a == 0
                        ? pixel.a
                        : static_cast<uint8_t>(layer_traversed.mOpacity)};

                displayed_canvas[i][j] =
                    Color::BlendColor(dst_color, displayed_canvas[i][j]);
            }
        }
    }

    return displayed_canvas;
}

void Layers::PushToHistory()
{
    auto& history = GetHistory();

    if (sCurrentCapture < history.size() - 1)
    {
        auto delete_begin = history.begin();
        std::advance(delete_begin, sCurrentCapture + 1);
        history.erase(delete_begin, history.end());
    }

    history.emplace_back(GetLayers(), sCurrentLayerIndex);

    if (history.size() > kMaxHistoryLenght)
    {
        auto delete_end =
            history.begin() +
            static_cast<std::ptrdiff_t>(history.size() - kMaxHistoryLenght);
        history.erase(history.begin(), delete_end);
    }

    sCurrentCapture = history.size() - 1;
}

void Layers::Undo()
{
    if (sCurrentCapture != 0)
    {
        sCurrentCapture--;
        auto& history = GetHistory();
        sCurrentLayerIndex = history[sCurrentCapture].selected_layer_index;
        VertexBufferControl::SetUpdateAllToTrue();
    }
}

void Layers::Redo()
{
    auto& history = GetHistory();

    if (sCurrentCapture != history.size() - 1)
    {
        sCurrentCapture++;
        sCurrentLayerIndex = history[sCurrentCapture].selected_layer_index;
        VertexBufferControl::SetUpdateAllToTrue();
    }
}

void Layers::UpdateAndDraw(bool should_do_tool)
{
    for (auto& layer : GetLayers())
    {
        layer.Update();
    }

    if (Project::IsOpened() && should_do_tool) { DoCurrentTool(); }

    if (Events::AreKeyboardKeysPressed(GLFW_KEY_LEFT_CONTROL, GLFW_KEY_Z))
    {
        Undo();
    }

    if (Events::AreKeyboardKeysPressed(GLFW_KEY_LEFT_CONTROL, GLFW_KEY_R))
    {
        Redo();
    }

    if (sShouldUpdateHistory) { PushToHistory(); }
    sShouldUpdateHistory = false;
}
} // namespace Pikzel
