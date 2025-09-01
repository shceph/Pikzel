#include "layer.hpp"

#include "camera.hpp"
#include "color.hpp"
#include "events.hpp"
#include "selection.hpp"
#include "tool.hpp"

#include "gla/pixel_buffer.hpp"
#include "gla/texture.hpp"

#include <glad/gl.h>

#include <GLFW/glfw3.h>

#include <imgui.h>

#include <glm/glm.hpp>

#include <algorithm>
#include <cassert>
#include <chrono>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <numbers>
#include <optional>
#include <queue>
#include <string>
#include <utility>
#include <vector>

namespace Pikzel {
auto Color::operator=(const ImVec4& color) -> Color& {
    r = static_cast<uint8_t>(color.x * UINT8_MAX);
    g = static_cast<uint8_t>(color.y * UINT8_MAX);
    b = static_cast<uint8_t>(color.z * UINT8_MAX);
    a = static_cast<uint8_t>(color.w * UINT8_MAX);
    return *this;
}

auto Color::operator==(const Color& other) const -> bool {
    return other.r == r && other.g == g && other.b == b && other.a == a;
}

auto Color::operator==(const ImVec4& other) const -> bool {
    constexpr float kTolerance = 0.0025F;

    return std::abs((static_cast<float>(r) / UINT8_MAX) - other.x) <=
               kTolerance &&
           std::abs((static_cast<float>(g) / UINT8_MAX) - other.y) <=
               kTolerance &&
           std::abs((static_cast<float>(b) / UINT8_MAX) - other.z) <=
               kTolerance &&
           std::abs((static_cast<float>(a) / UINT8_MAX) - other.w) <=
               kTolerance;
}

auto Color::Difference(Color other) const -> int {
    return std::abs(r - other.r) + std::abs(g - other.g) +
           std::abs(b - other.b) + std::abs(a - other.a);
}

auto Color::BlendColor(Color color1, Color color2) -> Color {
    const ImVec4 col1 = {
        static_cast<float>(color1.r) / UINT8_MAX,
        static_cast<float>(color1.g) / UINT8_MAX,
        static_cast<float>(color1.b) / UINT8_MAX,
        static_cast<float>(color1.a) / UINT8_MAX,
    };

    const ImVec4 col2 = {
        static_cast<float>(color2.r) / UINT8_MAX,
        static_cast<float>(color2.g) / UINT8_MAX,
        static_cast<float>(color2.b) / UINT8_MAX,
        static_cast<float>(color2.a) / UINT8_MAX,
    };

    const float alpha1 = col1.w;
    const float alpha2 = col2.w;
    const float out_alpha = alpha1 + (alpha2 * (1.0F - alpha1));

    if (out_alpha == 0) {
        return {.r = 0, .g = 0, .b = 0, .a = 0};
    }

    const float out_r =
        (col1.x * alpha1 + col2.x * alpha2 * (1.0F - alpha1)) / out_alpha;
    const float out_g =
        (col1.y * alpha1 + col2.y * alpha2 * (1.0F - alpha1)) / out_alpha;
    const float out_b =
        (col1.z * alpha1 + col2.z * alpha2 * (1.0F - alpha1)) / out_alpha;

    return Color::FromImVec4({out_r, out_g, out_b, out_alpha});
}

auto Color::FromImVec4(ImVec4 color) -> Color {
    return {.r = static_cast<uint8_t>(color.x * UINT8_MAX),
            .g = static_cast<uint8_t>(color.y * UINT8_MAX),
            .b = static_cast<uint8_t>(color.z * UINT8_MAX),
            .a = static_cast<uint8_t>(color.w * UINT8_MAX)};
}

auto Color::FromGlaColor(Gla::Color color) -> Color {
    return {.r = color.r, .g = color.g, .b = color.b, .a = color.a};
}

Layer::Layer(Tool& tool, Camera& camera, Selection& selection,
             Gla::PboMappedBuffSpan pbo_buff, Vec2 canvas_dims,
             bool is_canvas_layer /*= true*/) noexcept
    : mCanvasDims{canvas_dims}, mIsCanvasLayer{is_canvas_layer},
      mLayerName{"Layer " + std::to_string(sConstructCounter)}, mTool{tool},
      mCamera{camera}, mSelection{selection}, mPboBuff{pbo_buff},
      mTex{canvas_dims, {0.0F, 0.0F, 0.0F, 0.0F}, Gla::kNearest} {
    if (mIsCanvasLayer) {
        sConstructCounter++;
    }
}

auto Layer::DoCurrentTool(CanvasWindowData win_data)
    -> Layer::ShouldUpdateHistory {
    if (mLocked || !mVisible) {
        return false;
    }

    switch (mTool.get().GetToolType()) {
    case ToolType::kBrush:
    case ToolType::kEraser:
        return HandleBrushAndEraser(win_data);
        break;
    case ToolType::kColorPicker:
        HandleColorPicker(win_data);
        break;
    case ToolType::kBucket:
        return HandleBucket(win_data);
        break;

    // The following are handled by the LayerControl class.
    case ToolType::kRectShape:
    case ToolType::kSelectionTool:
    case ToolType::kColorSelection:
    case ToolType::kMoveSelection:
    case ToolType::kToolCount:
        assert(false);
    }

    return false;
}

void Layer::Update() { mIsEdited = false; }

auto Layer::HandleBrushAndEraser(CanvasWindowData win_data)
    -> Layer::ShouldUpdateHistory {
    static bool left_button_held = false;

    if (Events::IsMouseButtonHeld(Events::MouseButtons::kButtonLeft)) {
        left_button_held = true;
    }

    if (!Events::IsMouseButtonPressed(Events::MouseButtons::kButtonLeft)) {
        if (left_button_held) {
            left_button_held = false;
            return true;
        }

        return false;
    }

    auto canv_coord = CanvasCoordsFromCursorPos(win_data);
    if (!canv_coord.has_value()) {
        return false;
    }

    constexpr auto kMaxDelay = std::chrono::milliseconds(100);
    static auto time_last_drawn = std::chrono::steady_clock::now();
    static auto position_last_drawn = canv_coord.value();

    if (glm::distance<2, float>(glm::vec2(canv_coord.value()),
                                glm::vec2(position_last_drawn)) > 1 &&
        std::chrono::steady_clock::now() - time_last_drawn <= kMaxDelay) {
        const int thickness = mTool.get().GetBrushRadius() == 1
                                  ? 1
                                  : mTool.get().GetBrushRadius() * 2;

        if (mTool.get().GetToolType() == ToolType::kEraser) {
            DrawLine(canv_coord.value(), position_last_drawn, thickness,
                     Color{.r = 0, .g = 0, .b = 0, .a = 0});
        } else {
            DrawLine(canv_coord.value(), position_last_drawn, thickness);
        }
    } else {
        DrawCircle(canv_coord.value(), mTool.get().GetBrushRadius(),
                   DrawType::kFill);
    }

    time_last_drawn = std::chrono::steady_clock::now();
    position_last_drawn = canv_coord.value();
    return false;
}

void Layer::HandleColorPicker(CanvasWindowData win_data) {
    if (!Events::IsMouseButtonPressed(Events::MouseButtons::kButtonLeft)) {
        return;
    }
    auto canv_coord = CanvasCoordsFromCursorPos(win_data);
    if (!canv_coord.has_value()) {
        return;
    }

    auto picked_color = GetPixel(canv_coord.value());

    if (picked_color.a == 0) {
        return;
    }

    mTool.get().GetColorRef().x =
        static_cast<float>(picked_color.r) / UINT8_MAX;
    mTool.get().GetColorRef().y =
        static_cast<float>(picked_color.g) / UINT8_MAX;
    mTool.get().GetColorRef().z =
        static_cast<float>(picked_color.b) / UINT8_MAX;
}

auto Layer::HandleBucket(CanvasWindowData win_data)
    -> Layer::ShouldUpdateHistory {
    if (!Events::IsMouseButtonPressed(Events::MouseButtons::kButtonLeft)) {
        return false;
    }
    auto canv_coord = CanvasCoordsFromCursorPos(win_data);
    if (!canv_coord.has_value()) {
        return false;
    }

    const Color clicked_color = GetPixel(canv_coord.value());
    Fill(canv_coord->x, canv_coord->y, clicked_color);
    return true;
}

void Layer::DrawPixel(std::size_t index, Color color) {
    assert(std::cmp_less(index, mCanvasDims.x * mCanvasDims.y));

    if (mSelection.get().ShouldCheckForSelection()) {
        const std::vector<bool>& selected_pixels =
            mSelection.get().GetSelectedPixels();

        if (!selected_pixels[index]) {
            return;
        }
    }

    mPboBuff[index] = {
        .r = color.r,
        .g = color.g,
        .b = color.b,
        .a = color.a,
    };

    mIsEdited = true;
}

void Layer::DrawPixel(Vec2 coords) {
    DrawPixel(coords, Color::FromImVec4(mTool.get().GetColor()));
}

void Layer::DrawPixel(Vec2 coords, Color color) {
    if (!mSelection.get().IsPixelSelected(coords)) {
        return;
    }

    mPboBuff[(coords.y * mCanvasDims.x) + coords.x] = {
        .r = color.r,
        .g = color.g,
        .b = color.b,
        .a = color.a,
    };

    mIsEdited = true;
}

void Layer::DrawPixelClampCoords(Vec2 coords, Color color) {
    DrawPixel(ClampToCanvasDims(coords), color);
}

void Layer::DrawCircle(Vec2 center, int radius, DrawType draw_type,
                       Color delete_color /*= {0, 0, 0, 0}*/,
                       std::optional<Color> draw_color /*= std::nullopt*/) {
    if (radius < 1) {
        return;
    }

    Color draw_col = delete_color;

    if (mTool.get().GetToolType() != ToolType::kEraser) {
        draw_col =
            draw_color.value_or(Color::FromImVec4(mTool.get().GetColor()));
    }

    if (radius == 1) {
        DrawPixel(center, draw_col);
        return;
    }

    if (draw_type == DrawType::kFill) {
        for (int xcrd = -radius; xcrd <= radius; xcrd++) {
            for (int ycrd = -radius; ycrd <= radius; ycrd++) {
                if (xcrd * xcrd + ycrd * ycrd < radius * radius) {
                    const int real_x =
                        std::clamp(xcrd + center.x, 0, mCanvasDims.x - 1);
                    const int real_y =
                        std::clamp(ycrd + center.y, 0, mCanvasDims.y - 1);
                    DrawPixel({real_x, real_y}, draw_col);
                }
            }
        }

        return;
    }

    for (int x_coord = std::max(0, center.x - radius + 1);
         x_coord < std::min(mCanvasDims.x, center.x + radius); x_coord++) {
        const int x_relative = x_coord - center.x;
        double y1_coord =
            std::sqrt((radius * radius) - (x_relative * x_relative));
        double y2_coord = -y1_coord;
        y1_coord += center.y;
        y2_coord += center.y;

        // If the number is round floor and ceil don't change anything
        if (y1_coord == static_cast<int>(y1_coord)) {
            y1_coord--;
        }
        if (y2_coord == static_cast<int>(y2_coord)) {
            y2_coord++;
        }

        int y1_floor = std::floor(y1_coord);
        int y2_ceil = std::ceil(y2_coord);

        if (y1_floor < 0) {
            y1_floor = 0;
        } else if (y1_floor >= mCanvasDims.y) {
            y1_floor = mCanvasDims.y - 1;
        }

        if (y2_ceil < 0) {
            y2_ceil = 0;
        } else if (y2_ceil >= mCanvasDims.y) {
            y2_ceil = mCanvasDims.y - 1;
        }

        DrawPixel({x_coord, y1_floor}, draw_col);
        DrawPixel({x_coord, y2_ceil}, draw_col);
    }
}

void Layer::Clear() {
    for (int i = 0; i < mCanvasDims.y; i++) {
        for (int j = 0; j < mCanvasDims.x; j++) {
            DrawPixel({j, i}, {.r = 0, .g = 0, .b = 0, .a = 0});
        }
    }
}

void Layer::GetTextureData(std::vector<Color>& buffer) const {
    buffer.resize(static_cast<std::size_t>(mCanvasDims.x) * mCanvasDims.y);
    mTex.Bind();
    glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_UNSIGNED_BYTE, buffer.data());
    mTex.Unbind();
}

void Layer::UpdateMappedPBOBufferSpan(Gla::PboMappedBuffSpan pbo_buff) {
    mPboBuff = pbo_buff;
}

void Layer::DrawRect(Vec2 upper_left, Vec2 bottom_right, DrawType /*draw_type*/,
                     std::optional<Color> color /*= std::nullopt*/) {
    const Color col = color.value_or(Color::FromImVec4(mTool.get().GetColor()));

    const std::size_t max_x = std::max(upper_left.x, bottom_right.x);
    const std::size_t min_x = std::min(upper_left.x, bottom_right.x);
    const std::size_t max_y = std::max(upper_left.y, bottom_right.y);
    const std::size_t min_y = std::min(upper_left.y, bottom_right.y);

    for (auto i = min_y; i <= max_y; i++) {
        for (auto j = min_x; j <= max_x; j++) {
            // mCanvas[i][j] = Tool::GetColor();
            DrawPixel({j, i}, col);
        }
    }
}

void Layer::DrawThickLine(Vec2 point_a, Vec2 point_b, int thickness,
                          Color color) {
    const Vec2 diff = point_a - point_b;
    auto angle = std::atan2(diff.y, diff.x) + (std::numbers::pi / 2);
    auto angle_plus_180 = angle + std::numbers::pi;

    Vec2 point_a1;
    point_a1.x =
        static_cast<int>(std::cos(angle) * (static_cast<float>(thickness) / 2));
    point_a1.y =
        static_cast<int>(std::sin(angle) * (static_cast<float>(thickness) / 2));
    point_a1 += point_a;
    point_a1 = ClampToCanvasDims(point_a1);

    Vec2 point_a2;
    point_a2.x = static_cast<int>(std::cos(angle_plus_180) *
                                  (static_cast<float>(thickness) / 2));
    point_a2.y = static_cast<int>(std::sin(angle_plus_180) *
                                  (static_cast<float>(thickness) / 2));
    point_a2 += point_a;
    point_a2 = ClampToCanvasDims(point_a2);

    Vec2 point_b1;
    point_b1.x =
        static_cast<int>(std::cos(angle) * (static_cast<float>(thickness) / 2));
    point_b1.y =
        static_cast<int>(std::sin(angle) * (static_cast<float>(thickness) / 2));
    point_b1 += point_b;
    point_b1 = ClampToCanvasDims(point_b1);

    Vec2 point_b2;
    point_b2.x = static_cast<int>(std::cos(angle_plus_180) *
                                  (static_cast<float>(thickness) / 2));
    point_b2.y = static_cast<int>(std::sin(angle_plus_180) *
                                  (static_cast<float>(thickness) / 2));
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

    const Vec2 line_middle = (point_a1 + point_a2 + point_b1 + point_b2) / 4;
    FillUntil(kOutlineColor, line_middle.x, line_middle.y, color);

    DrawLine(point_a1, point_a2, color);
    DrawLine(point_b1, point_b2, color);
    DrawLine(point_a1, point_b1, color);
    DrawLine(point_a2, point_b2, color);
    DrawPixel(point_a1, color);
    DrawPixel(point_a2, color);
    DrawPixel(point_b1, color);
    DrawPixel(point_b2, color);

    DrawCircle(point_a, thickness / 2, DrawType::kFill);
    DrawCircle(point_b, thickness / 2, DrawType::kFill);
}

void Layer::DrawLine(Vec2 point_a, Vec2 point_b, int thickness,
                     std::optional<Color> color /*= std::nullopt*/) {
    Color col = color.value_or(Color::FromImVec4(mTool.get().GetColor()));

    if (thickness == 1) {
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

    const bool steep = abs(y_1 - y_0) > abs(x_1 - x_0);
    if (steep) {
        std::swap(x_0, y_0);
        std::swap(x_1, y_1);
    }

    if (x_0 > x_1) {
        std::swap(x_0, x_1);
        std::swap(y_0, y_1);
    }

    const int d_x = x_1 - x_0;
    const int d_y = abs(y_1 - y_0);
    int error = d_x / 2;
    const int y_step = (y_0 < y_1) ? 1 : -1;
    int y_coord = y_0;

    const int offset = thickness / 2;

    for (int x_coord = x_0; x_coord <= x_1; ++x_coord) {
        const int draw_x = steep ? y_coord : x_coord;
        const int draw_y = steep ? x_coord : y_coord;

        // Draw the thick line by offsetting the perpendicular direction
        for (int i = -offset + 1; i < offset; ++i) {
            if (steep) {
                DrawPixel(ClampToCanvasDims({draw_x + i, draw_y}), col);
            } else {
                DrawPixel(ClampToCanvasDims({draw_x, draw_y + i}), col);
            }
        }

        error -= d_y;
        if (error < 0) {
            y_coord += y_step;
            error += d_x;
        }
    }
}

void Layer::DrawLine(Vec2 point_a, Vec2 point_b,
                     std::optional<Color> color /*= std::nullopt*/) {
    const Color draw_color =
        color.value_or(Color::FromImVec4(mTool.get().GetColor()));

    const int diff_x = std::abs(point_a.x - point_b.x);
    const int diff_y = std::abs(point_a.y - point_b.y);
    const int sign_x = (point_a.x < point_b.x) ? 1 : -1;
    const int sign_y = (point_a.y < point_b.y) ? 1 : -1;
    int err = diff_x - diff_y;

    while (point_a != point_b) {
        const int err2 = err;

        if (err2 > -diff_y) {
            err -= diff_y;
            point_a.x += sign_x;
        }

        if (err2 < diff_x) {
            err += diff_x;
            point_a.y += sign_y;
        }

        DrawPixel(point_a, draw_color);
    }
}

void Layer::Fill(int x_coord, int y_coord, Color clicked_color) {
    Fill(x_coord, y_coord, clicked_color,
         Color::FromImVec4(mTool.get().GetColor()));
}

void Layer::Fill(int x_coord, int y_coord, Color clicked_color,
                 Color fill_color) {
    if (x_coord < 0 || x_coord >= mCanvasDims.x || y_coord < 0 ||
        y_coord >= mCanvasDims.y) {
        return;
    }

    std::queue<std::pair<int, int>> pixel_queue;
    pixel_queue.emplace(y_coord, x_coord);

    while (!pixel_queue.empty()) {
        auto& top = pixel_queue.front();
        const int row = top.first;
        const int col = top.second;
        const Color pixel = GetPixel({col, row});

        if (pixel == clicked_color && pixel != fill_color) {
            DrawPixel({col, row}, fill_color);

            if (col + 1 < mCanvasDims.x) {
                pixel_queue.emplace(row, col + 1);
            }

            if (col - 1 >= 0) {
                pixel_queue.emplace(row, col - 1);
            }

            if (row + 1 < mCanvasDims.y) {
                pixel_queue.emplace(row + 1, col);
            }

            if (row - 1 >= 0) {
                pixel_queue.emplace(row - 1, col);
            }
        }

        pixel_queue.pop();
    }
}

void Layer::FillUntil(Color until_color, int x_coord, int y_coord,
                      Color fill_color) {
    if (x_coord < 0 || x_coord >= mCanvasDims.x || y_coord < 0 ||
        y_coord >= mCanvasDims.y) {
        return;
    }

    std::queue<std::pair<int, int>> pixel_queue;
    pixel_queue.emplace(y_coord, x_coord);

    std::vector<bool> visited(
        static_cast<std::size_t>(mCanvasDims.x * mCanvasDims.y), false);

    while (!pixel_queue.empty()) {
        auto& top = pixel_queue.front();
        const int row = top.first;
        const int col = top.second;
        const Color pixel = GetPixel({col, row});

        if (pixel != until_color && !visited[(row * mCanvasDims.x) + col] &&
            mSelection.get().IsPixelSelected({col, row})) {
            DrawPixelClampCoords({col, row}, fill_color);
            visited[(row * mCanvasDims.x) + col] = true;

            if (col + 1 < mCanvasDims.x) {
                pixel_queue.emplace(row, col + 1);
            }

            if (col - 1 >= 0) {
                pixel_queue.emplace(row, col - 1);
            }

            if (row + 1 < mCanvasDims.y) {
                pixel_queue.emplace(row + 1, col);
            }

            if (row - 1 >= 0) {
                pixel_queue.emplace(row - 1, col);
            }
        }

        pixel_queue.pop();
    }
}

auto Layer::CanvasCoordsFromCursorPos(CanvasWindowData win_data) const
    -> std::optional<Vec2> {
    double cursor_x = NAN;
    double cursor_y = NAN;
    // Cursor position relative to the Glfw window
    glfwGetCursorPos(win_data.window, &cursor_x, &cursor_y);

    // Wayland doesn't give window position info to client apps
    if (glfwGetPlatform() != GLFW_PLATFORM_WAYLAND) {
        int window_x = 0;
        int window_y = 0;
        // Position of the window relative to the screen
        glfwGetWindowPos(win_data.window, &window_x, &window_y);

        cursor_x += window_x; // Getting cursor position relative to the screen
        cursor_y += window_y;
    }

    const ImVec2 canvas_upperleft = win_data.win_upper_left;
    const ImVec2 canvas_bottomtright = win_data.win_bottom_right;

    if (cursor_x <= canvas_upperleft.x || cursor_x >= canvas_bottomtright.x ||
        cursor_y <= canvas_upperleft.y || cursor_y >= canvas_bottomtright.y) {
        return std::nullopt;
    }

    const glm::vec2 canvas_dims_flt{mCanvasDims};
    const glm::vec2 cursor_draw_win_relative{cursor_x - canvas_upperleft.x,
                                             cursor_y - canvas_upperleft.y};
    const glm::vec2 canvas_on_screen_dims{
        canvas_bottomtright.x - canvas_upperleft.x,
        canvas_bottomtright.y - canvas_upperleft.y};

    glm::vec2 coords =
        cursor_draw_win_relative / (canvas_on_screen_dims / canvas_dims_flt);
    const double zoom_val = mCamera.get().GetZoomValue();

    if (zoom_val != 0) {
        const float inv_zoom = 1.0F - static_cast<float>(zoom_val);
        const float new_width = canvas_dims_flt.x * inv_zoom;
        const float new_height = canvas_dims_flt.y * inv_zoom;
        coords.x = coords.x * inv_zoom;
        coords.y = coords.y * inv_zoom;
        coords.x += (canvas_dims_flt.x - new_width) / 2;
        coords.y += (canvas_dims_flt.y - new_height) / 2;
    }

    coords += mCamera.get().GetCenterAsVec2Int() - mCanvasDims / 2;

    if (coords.x < 0 || coords.x >= canvas_dims_flt.x || coords.y < 0 ||
        coords.y >= canvas_dims_flt.y) {
        return std::nullopt;
    }

    return std::make_optional(coords);
}

auto Layer::ClampToCanvasDims(Vec2 val_to_clamp) -> Vec2 {
    return glm::clamp(val_to_clamp, {0, 0}, mCanvasDims - 1);
}
} // namespace Pikzel
