#include "layer.hpp"

#include "application.hpp"
#include "events.hpp"
#include "project.hpp"
#include "tool.hpp"

#include "GLFW/glfw3.h"
#include <cstddef>
#include <glm/geometric.hpp>

#include <algorithm>
#include <chrono>
#include <cmath>
#include <queue>
#include <utility>
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

    return std::abs((static_cast<float>(r) / 0xff) - other.x) <= kTolerance &&
           std::abs((static_cast<float>(g) / 0xff) - other.y) <= kTolerance &&
           std::abs((static_cast<float>(b) / 0xff) - other.z) <= kTolerance &&
           std::abs((static_cast<float>(a) / 0xff) - other.w) <= kTolerance;
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
    float out_alpha = alpha1 + (alpha2 * (1.0F - alpha1));

    if (out_alpha == 0) { return {.r = 0, .g = 0, .b = 0, .a = 0}; }

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
    return {.r = static_cast<uint8_t>(color.x * 0xff),
            .g = static_cast<uint8_t>(color.y * 0xff),
            .b = static_cast<uint8_t>(color.z * 0xff),
            .a = static_cast<uint8_t>(color.w * 0xff)};
}

Layer::Layer(std::shared_ptr<Tool> tool, std::shared_ptr<Camera> camera,
             Vec2Int canvas_dims, bool is_canvas_layer /*= true*/,
             bool draw_visible_pixels_only /*= false*/) noexcept
    : mCanvas{static_cast<std::size_t>(canvas_dims.x * canvas_dims.y)},
      mCanvasDims{canvas_dims}, mIsCanvasLayer{is_canvas_layer},
      mDrawVisiblePixelsOnly{draw_visible_pixels_only},
      mLayerName{"Layer " + std::to_string(sConstructCounter)},
      mTool{std::move(tool)}, mCamera{std::move(camera)}
{
    if (mIsCanvasLayer) { sConstructCounter++; }
}

auto Layer::DoCurrentTool() -> Layer::ShouldUpdateHistory
{
    if (mLocked || !mVisible) { return false; }

    switch (mTool->GetToolType())
    {
    case ToolType::kBrush:
    case ToolType::kEraser:
        return HandleBrushAndEraser();
        break;
    case ToolType::kColorPicker:
        HandleColorPicker();
        break;
    case ToolType::kBucket:
        return HandleBucket();
        break;
    case ToolType::kRectShape:
        return HandleRectShape();
        break;
    case ToolType::kToolCount:
        assert(false);
    }

    return false;
}

void Layer::EmplaceVertices(std::vector<Vertex>& vertices,
                            bool use_color_alpha /*= false*/) const
{
    if (!mVisible || mOpacity == 0) { return; }

    for (int i = 0; i < mCanvasDims.y; i++)
    {
        for (int j = 0; j < mCanvasDims.x; j++)
        {
            auto pixel_color = GetPixel({j, i});

            if (mDrawVisiblePixelsOnly && pixel_color.a == 0) { continue; }

            uint8_t alpha_val = 0;
            if (use_color_alpha) { alpha_val = pixel_color.a; }
            else { alpha_val = mOpacity; }

            Color color_used = pixel_color;

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

auto Layer::HandleBrushAndEraser() -> Layer::ShouldUpdateHistory
{
    static bool left_button_held = false;

    if (Events::IsMouseButtonHeld(Events::MouseButtons::kButtonLeft))
    {
        left_button_held = true;
    }

    if (!Events::IsMouseButtonPressed(Events::MouseButtons::kButtonLeft))
    {
        if (left_button_held)
        {
            left_button_held = false;
            return true;
        }

        return false;
    }

    auto canv_coord = CanvasCoordsFromCursorPos();
    if (!canv_coord.has_value()) { return false; }

    constexpr auto kMaxDelay = std::chrono::milliseconds(100);
    static auto time_last_drawn = std::chrono::steady_clock::now();
    static auto position_last_drawn = canv_coord.value();

    if (glm::distance<2, float>(glm::vec2(canv_coord.value()),
                                glm::vec2(position_last_drawn)) > 1 &&
        std::chrono::steady_clock::now() - time_last_drawn <= kMaxDelay)
    {
        int thickness =
            mTool->GetBrushRadius() == 1 ? 1 : mTool->GetBrushRadius() * 2;

        if (mTool->GetToolType() == ToolType::kEraser)
        {
            DrawLine(canv_coord.value(), position_last_drawn, thickness,
                     Color{.r = 0, .g = 0, .b = 0, .a = 0});
        }
        else { DrawLine(canv_coord.value(), position_last_drawn, thickness); }
    }
    else { DrawCircle(canv_coord.value(), mTool->GetBrushRadius(), true); }

    time_last_drawn = std::chrono::steady_clock::now();
    position_last_drawn = canv_coord.value();
    return false;
}

void Layer::HandleColorPicker()
{
    if (!Events::IsMouseButtonPressed(Events::MouseButtons::kButtonLeft))
    {
        return;
    }
    auto canv_coord = CanvasCoordsFromCursorPos();
    if (!canv_coord.has_value()) { return; }

    auto picked_color = GetPixel(canv_coord.value());

    if (picked_color.a == 0) { return; }

    mTool->GetColorRef().x = static_cast<float>(picked_color.r) / 0xff;
    mTool->GetColorRef().y = static_cast<float>(picked_color.g) / 0xff;
    mTool->GetColorRef().z = static_cast<float>(picked_color.b) / 0xff;
}

auto Layer::HandleBucket() -> Layer::ShouldUpdateHistory
{
    if (!Events::IsMouseButtonPressed(Events::MouseButtons::kButtonLeft))
    {
        return false;
    }
    auto canv_coord = CanvasCoordsFromCursorPos();
    if (!canv_coord.has_value()) { return false; }

    Color clicked_color = GetPixel(canv_coord.value());
    Fill(canv_coord->x, canv_coord->y, clicked_color);
    return true;
}

auto Layer::HandleRectShape() -> Layer::ShouldUpdateHistory
{
    auto canv_coord = CanvasCoordsFromCursorPos();
    if (!canv_coord.has_value()) { return false; }
    bool left_button_pressed =
        Events::IsMouseButtonPressed(Events::MouseButtons::kButtonLeft);

    /* static bool shape_began = false; */
    /* static Vec2Int shape_begin_coords{0, 0}; */

    if (!mHandleRectShapeData.shape_began)
    {
        if (left_button_pressed)
        {
            mHandleRectShapeData.shape_begin_coords = canv_coord.value();
            mHandleRectShapeData.shape_began = true;
        }
        return false;
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

        return false;
    }

    if (mIsCanvasLayer)
    {
        DrawRect(mHandleRectShapeData.shape_begin_coords, canv_coord.value(),
                 true);
    }
    else { Clear(); }

    mHandleRectShapeData.shape_began = false;
    return true;
}

void Layer::DrawPixel(Vec2Int coords)
{
    DrawPixel(coords, Color::FromImVec4(mTool->GetColor()));
}

void Layer::DrawPixel(Vec2Int coords, Color color)
{
    std::unique_lock<std::mutex> lock{sMutex};
    mCanvas[(coords.y * mCanvasDims.x) + coords.x] = color;
    lock.unlock();

    if (mIsCanvasLayer) { GetDirtyPixels().push_back(coords); }
}

void Layer::DrawPixelClampCoords(Vec2Int coords, Color color)
{
    DrawPixel(ClampToCanvasDims(coords), color);
}

void Layer::DrawCircle(Vec2Int center, int radius, bool fill,
                       Color delete_color /*= {0, 0, 0, 0}*/)
{
    if (radius < 1) { return; }

    Color draw_color = delete_color;

    if (mTool->GetToolType() != ToolType::kEraser)
    {
        draw_color = mTool->GetColor();
        draw_color.a = 0xff;
    }

    if (radius == 1)
    {
        DrawPixel(center, draw_color);
        return;
    }

    if (fill)
    {
        for (int xcrd = -radius; xcrd <= radius; xcrd++)
        {
            for (int ycrd = -radius; ycrd <= radius; ycrd++)
            {
                if (xcrd * xcrd + ycrd * ycrd < radius * radius)
                {
                    int real_x =
                        std::clamp(xcrd + center.x, 0, mCanvasDims.x - 1);
                    int real_y =
                        std::clamp(ycrd + center.y, 0, mCanvasDims.y - 1);
                    DrawPixel({real_x, real_y}, draw_color);
                }
            }
        }

        return;
    }

    for (int x_coord = std::max(0, center.x - radius + 1);
         x_coord < std::min(mCanvasDims.x, center.x + radius); x_coord++)
    {
        int x_relative = x_coord - center.x;
        double y1_coord =
            std::sqrt((radius * radius) - (x_relative * x_relative));
        double y2_coord = -y1_coord;
        y1_coord += center.y;
        y2_coord += center.y;

        // If the number is round floor and ceil don't change anything
        if (y1_coord == static_cast<int>(y1_coord)) { y1_coord--; }
        if (y2_coord == static_cast<int>(y2_coord)) { y2_coord++; }

        int y1_floor = std::floor(y1_coord);
        int y2_ceil = std::ceil(y2_coord);

        if (y1_floor < 0) { y1_floor = 0; }
        else if (y1_floor >= mCanvasDims.y) { y1_floor = mCanvasDims.y - 1; }

        if (y2_ceil < 0) { y2_ceil = 0; }
        else if (y2_ceil >= mCanvasDims.y) { y2_ceil = mCanvasDims.y - 1; }

        /* mCanvas[y1_floor][x_coord] = draw_color; */
        /* mCanvas[y2_ceil][x_coord] = draw_color; */
        DrawPixel({x_coord, y1_floor}, draw_color);
        DrawPixel({x_coord, y2_ceil}, draw_color);
    }
}

void Layer::Clear()
{
    for (int i = 0; i < mCanvasDims.y; i++)
    {
        for (int j = 0; j < mCanvasDims.x; j++)
        {
            DrawPixel({j, i}, {.r = 0, .g = 0, .b = 0, .a = 0});
        }
    }
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

void Layer::DrawThickLine(Vec2Int point_a, Vec2Int point_b, int thickness,
                          Color color)
{
    Vec2Int diff = point_a - point_b;
    auto angle = std::atan2(diff.y, diff.x) + M_PI_2;
    auto angle_plus_180 = angle + M_PI;

    Vec2Int point_a1;
    point_a1.x = static_cast<int>(std::cos(angle) *
                                  (static_cast<float>(thickness) / 2.0));
    point_a1.y = static_cast<int>(std::sin(angle) *
                                  (static_cast<float>(thickness) / 2.0));
    point_a1 += point_a;
    point_a1 = ClampToCanvasDims(point_a1);

    Vec2Int point_a2;
    point_a2.x = static_cast<int>(std::cos(angle_plus_180) *
                                  (static_cast<float>(thickness) / 2.0));
    point_a2.y = static_cast<int>(std::sin(angle_plus_180) *
                                  (static_cast<float>(thickness) / 2.0));
    point_a2 += point_a;
    point_a2 = ClampToCanvasDims(point_a2);

    Vec2Int point_b1;
    point_b1.x = static_cast<int>(std::cos(angle) *
                                  (static_cast<float>(thickness) / 2.0));
    point_b1.y = static_cast<int>(std::sin(angle) *
                                  (static_cast<float>(thickness) / 2.0));
    point_b1 += point_b;
    point_b1 = ClampToCanvasDims(point_b1);

    Vec2Int point_b2;
    point_b2.x = static_cast<int>(std::cos(angle_plus_180) *
                                  (static_cast<float>(thickness) / 2.0));
    point_b2.y = static_cast<int>(std::sin(angle_plus_180) *
                                  (static_cast<float>(thickness) / 2.0));
    point_b2 += point_b;
    point_b2 = ClampToCanvasDims(point_b2);

    constexpr Color kOutlineColor = {.r = 253, .g = 254, .b = 255, .a = 255};
    DrawLine(point_a1, point_a2, kOutlineColor);
    DrawLine(point_b1, point_b2, kOutlineColor);
    DrawLine(point_a1, point_b1, kOutlineColor);
    DrawLine(point_a2, point_b2, kOutlineColor);
    DrawPixel(point_a1, kOutlineColor);
    DrawPixel(point_a2, kOutlineColor);
    DrawPixel(point_b1, kOutlineColor);
    DrawPixel(point_b2, kOutlineColor);

    Vec2Int line_middle = (point_a1 + point_a2 + point_b1 + point_b2) / 4;
    FillUntil(kOutlineColor, line_middle.x, line_middle.y, color);

    DrawLine(point_a1, point_a2, color);
    DrawLine(point_b1, point_b2, color);
    DrawLine(point_a1, point_b1, color);
    DrawLine(point_a2, point_b2, color);
    DrawPixel(point_a1, color);
    DrawPixel(point_a2, color);
    DrawPixel(point_b1, color);
    DrawPixel(point_b2, color);

    DrawCircle(point_a, thickness / 2, true);
    DrawCircle(point_b, thickness / 2, true);
}

void Layer::DrawLine(Vec2Int point_a, Vec2Int point_b, int thickness,
                     std::optional<Color> color /*= std::nullopt*/)
{
    Color col = color.value_or(Color::FromImVec4(mTool->GetColor()));

    if (thickness == 1)
    {
        DrawPixel(point_a, col);
        DrawPixel(point_b, col);
        DrawLine(point_a, point_b, col);
        return;
    }

    DrawThickLine(point_a, point_b, thickness, col);
    return;

    int x_0 = point_a.x;
    int y_0 = point_a.y;
    int x_1 = point_b.x;
    int y_1 = point_b.y;

    bool steep = abs(y_1 - y_0) > abs(x_1 - x_0);
    if (steep)
    {
        std::swap(x_0, y_0);
        std::swap(x_1, y_1);
    }

    if (x_0 > x_1)
    {
        std::swap(x_0, x_1);
        std::swap(y_0, y_1);
    }

    int d_x = x_1 - x_0;
    int d_y = abs(y_1 - y_0);
    int error = d_x / 2;
    int y_step = (y_0 < y_1) ? 1 : -1;
    int y_coord = y_0;

    int offset = thickness / 2;

    for (int x_coord = x_0; x_coord <= x_1; ++x_coord)
    {
        int draw_x = steep ? y_coord : x_coord;
        int draw_y = steep ? x_coord : y_coord;

        // Draw the thick line by offsetting the perpendicular direction
        for (int i = -offset + 1; i < offset; ++i)
        {
            if (steep)
            {
                DrawPixel(ClampToCanvasDims({draw_x + i, draw_y}), col);
            }
            else { DrawPixel(ClampToCanvasDims({draw_x, draw_y + i}), col); }
        }

        error -= d_y;
        if (error < 0)
        {
            y_coord += y_step;
            error += d_x;
        }
    }
}

void Layer::DrawLine(Vec2Int point_a, Vec2Int point_b,
                     std::optional<Color> color /*= std::nullopt*/)
{
    Color draw_color = color.value_or(Color::FromImVec4(mTool->GetColor()));

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

        DrawPixel(point_a, draw_color);
    }
}

void Layer::Fill(int x_coord, int y_coord, Color clicked_color)
{
    Fill(x_coord, y_coord, clicked_color, Color::FromImVec4(mTool->GetColor()));
}

void Layer::Fill(int x_coord, int y_coord, Color clicked_color,
                 Color fill_color)
{
    if (x_coord < 0 || x_coord >= mCanvasDims.x || y_coord < 0 ||
        y_coord >= mCanvasDims.y)
    {
        return;
    }

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
            DrawPixel({col, row}, fill_color);

            if (col + 1 < mCanvasDims.x) { pixel_queue.emplace(row, col + 1); }

            if (col - 1 >= 0) { pixel_queue.emplace(row, col - 1); }

            if (row + 1 < mCanvasDims.y) { pixel_queue.emplace(row + 1, col); }

            if (row - 1 >= 0) { pixel_queue.emplace(row - 1, col); }
        }

        pixel_queue.pop();
    }
}

void Layer::FillUntil(Color until_color, int x_coord, int y_coord,
                      Color fill_color)
{
    if (x_coord < 0 || x_coord >= mCanvasDims.x || y_coord < 0 ||
        y_coord >= mCanvasDims.y)
    {
        return;
    }

    std::queue<std::pair<int, int>> pixel_queue;
    pixel_queue.emplace(y_coord, x_coord);

    std::vector<bool> visited(
        static_cast<std::size_t>(mCanvasDims.x * mCanvasDims.y), false);

    while (!pixel_queue.empty())
    {
        auto& top = pixel_queue.front();
        const int row = top.first;
        const int col = top.second;
        Color pixel = GetPixel({col, row});

        if (pixel != until_color && !visited[(row * mCanvasDims.x) + col])
        {
            DrawPixelClampCoords({col, row}, fill_color);
            visited[(row * mCanvasDims.x) + col] = true;

            if (col + 1 < mCanvasDims.x) { pixel_queue.emplace(row, col + 1); }

            if (col - 1 >= 0) { pixel_queue.emplace(row, col - 1); }

            if (row + 1 < mCanvasDims.y) { pixel_queue.emplace(row + 1, col); }

            if (row - 1 >= 0) { pixel_queue.emplace(row - 1, col); }
        }

        pixel_queue.pop();
    }
}

auto Layer::CanvasCoordsFromCursorPos() const -> std::optional<Vec2Int>
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

    Vec2Int coords = cursor_draw_win_relative /
                     (canvas_on_screen_dims / glm::vec2{mCanvasDims});
    double zoom_val = mCamera->GetZoomValue();

    if (zoom_val != 0)
    {
        double inv_zoom = 1 - zoom_val;
        int new_width = static_cast<int>(mCanvasDims.x * inv_zoom);
        int new_height = static_cast<int>(mCanvasDims.y * inv_zoom);
        coords.x = static_cast<int>(coords.x * inv_zoom);
        coords.y = static_cast<int>(coords.y * inv_zoom);
        coords.x += (mCanvasDims.x - new_width) / 2;
        coords.y += (mCanvasDims.y - new_height) / 2;
    }

    coords += mCamera->GetCenterAsVec2Int() - mCanvasDims / 2;

    if (coords.x < 0 || coords.x >= mCanvasDims.x || coords.y < 0 ||
        coords.y >= mCanvasDims.y)
    {
        return std::nullopt;
    }

    return std::make_optional(coords);
}

auto Layer::ClampToCanvasDims(Vec2Int val_to_clamp) -> Vec2Int
{
    return glm::clamp(val_to_clamp, {0, 0}, mCanvasDims - 1);
}

// Should call this func before VertexBufferControl::Update, since it needs
// dirty pixels
void Layer::ResetDirtyPixelData()
{
    sShouldUpdateWholeVBO = false;
    GetDirtyPixels().clear();
}
} // namespace Pikzel
