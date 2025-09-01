#pragma once

#include "gla/pixel_buffer.hpp"

#include <imgui.h>

#include <cstdint>

namespace Pikzel {
struct Color {
    auto operator=(const ImVec4& color) -> Color&;
    auto operator==(const Color& other) const -> bool;
    auto operator==(const ImVec4& other) const -> bool;

    [[nodiscard]] auto Difference(Color other) const -> int;

    static auto BlendColor(Color color1, Color color2) -> Color;
    static auto FromImVec4(ImVec4 color) -> Color;
    static auto FromGlaColor(Gla::Color color) -> Color;

    uint8_t r = 0, g = 0, b = 0, a = 0;
};

struct ColorConstants {
    static constexpr Color kColorTransparent{.r = 0, .g = 0, .b = 0, .a = 0};
    static constexpr Color kColorSelectionPreview{
        .r = 45, .g = 50, .b = 220, .a = 100};
    static constexpr Color kMoveSelectionPreview{
        .r = 134, .g = 13, .b = 34, .a = 128};
};
} // namespace Pikzel
