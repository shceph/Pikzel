#pragma once

#include <glad/gl.h>
#include <GLFW/glfw3.h>

#include "camera.hpp"
#include "selection.hpp"
#include "tool.hpp"

#include <imgui.h>

#include <cstdint>
#include <cstddef>
#include <cassert>
#include <vector>
#include <functional>
#include <optional>
#include <string>
#include <utility>

#include "gla/pixel_buffer.hpp"
#include "gla/texture.hpp"

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

static constexpr Color kColorTransparent{.r = 0, .g = 0, .b = 0, .a = 0};
static constexpr Color kColorSelectionPreview{
    .r = 45, .g = 50, .b = 220, .a = 100};

struct Vertex {
    float pos_x{}, pos_y{};
    Color color{.r = 0, .g = 0, .b = 0, .a = 0};
};

class Layer {
  public:
    struct RectShapeData {
        bool shape_began = false;
        Vec2 shape_begin_coords{0, 0};
    };

    struct CanvasWindowData {
        ImVec2 win_upper_left;
        ImVec2 win_bottom_right;
        GLFWwindow* window{nullptr};
    };

    enum DrawType : uint8_t { kFill, kOutlline };

    explicit Layer(Tool& tool, Camera& camera, Selection& selection,
                   Gla::PboMappedBuffSpan pbo_buff, Vec2 canvas_dims,
                   bool is_canvas_layer = true) noexcept;

    using ShouldUpdateHistory = bool;
    auto DoCurrentTool(CanvasWindowData win_data) -> ShouldUpdateHistory;
    void Update();

    void SwitchVisibilityState() { mVisible = !mVisible; }
    void SwitchLockState() { mLocked = !mLocked; }

    [[nodiscard]] auto IsEdited() const -> bool { return mIsEdited; }
    [[nodiscard]] auto IsVisible() const -> bool { return mVisible; }
    [[nodiscard]] auto IsLocked() const -> bool { return mLocked; }
    [[nodiscard]] auto GetOpacity() const -> int { return mOpacity; }
    [[nodiscard]] auto GetName() const -> const std::string& {
        return mLayerName;
    }
    [[nodiscard]] auto GetPixel(Vec2 coords) const -> Color {
        auto col = mPboBuff[(coords.y * mCanvasDims.x) + coords.x];
        return {.r = col.r, .g = col.g, .b = col.b, .a = col.a};
    }
    [[nodiscard]] auto GetPixel(std::size_t index) const -> Color {
        assert(std::cmp_less(index, mCanvasDims.x * mCanvasDims.y));
        auto col = mPboBuff[index];
        return {.r = col.r, .g = col.g, .b = col.b, .a = col.a};
    }
    [[nodiscard]] auto GetCanvasDims() const -> Vec2 { return mCanvasDims; }
    [[nodiscard]] auto IsPreviewLayer() const -> bool {
        return !mIsCanvasLayer;
    }
    [[nodiscard]] auto GetTexture() const -> const Gla::Texture2D& {
        return mTex;
    }
    [[nodiscard]] auto GetTexture() -> Gla::Texture2D& { return mTex; }

    void SetOpacity(int opacity) { mOpacity = opacity; }

    // Returns Vec2Int if the cursor is above canvas, otherwise returns
    // std::nullopt
    [[nodiscard]]
    auto CanvasCoordsFromCursorPos(CanvasWindowData win_data) const
        -> std::optional<Vec2>;
    auto ClampToCanvasDims(Vec2 val_to_clamp) -> Vec2;

    static void ResetConstructCounter() { sConstructCounter = 1; }

    // Custom delete color can be set, I'm using this for the preview layer
    // where I want the brush to have a specific color when I'm using eraser.
    void DrawCircle(Vec2 center, int radius, DrawType draw_type,
                    Color delete_color = {.r = 0, .g = 0, .b = 0, .a = 0},
                    std::optional<Color> draw_color = std::nullopt);
    void Clear();
    void GetTextureData(std::vector<Color>& buffer) const;
    void UpdateMappedPBOBufferSpan(Gla::PboMappedBuffSpan pbo_buff);

  private:
    auto HandleBrushAndEraser(CanvasWindowData win_data) -> ShouldUpdateHistory;
    void HandleColorPicker(CanvasWindowData win_data);
    auto HandleBucket(CanvasWindowData win_data) -> ShouldUpdateHistory;

    void DrawPixel(std::size_t index, Color color);
    void DrawPixel(Vec2 coords);
    void DrawPixel(Vec2 coords, Color color);
    void DrawPixelClampCoords(Vec2 coords, Color color);
    void DrawRect(Vec2 upper_left, Vec2 bottom_right, DrawType draw_type,
                  std::optional<Color> color = std::nullopt);
    void DrawThickLine(Vec2 point_a, Vec2 point_b, int thickness, Color color);
    void DrawLine(Vec2 point_a, Vec2 point_b, int thickness,
                  std::optional<Color> color = std::nullopt);
    void DrawLine(Vec2 point_a, Vec2 point_b,
                  std::optional<Color> color = std::nullopt);
    void Fill(int x_coord, int y_coord, Color clicked_color);
    void Fill(int x_coord, int y_coord, Color clicked_color, Color fill_color);
    void FillUntil(Color until_color, int x_coord, int y_coord,
                   Color fill_color);

    Vec2 mCanvasDims;
    bool mIsCanvasLayer;
    bool mIsEdited{false};
    bool mVisible{true};
    bool mLocked{false};
    int mOpacity{255};
    std::string mLayerName;
    std::reference_wrapper<Tool> mTool;
    std::reference_wrapper<Camera> mCamera;
    std::reference_wrapper<Selection> mSelection;
    Gla::PboMappedBuffSpan mPboBuff;
    Gla::Texture2D mTex;

    inline static int sConstructCounter{1};

    friend class UI;
    friend class LayerControl;
    friend class PreviewLayer;
    friend class Project;
};
} // namespace Pikzel
