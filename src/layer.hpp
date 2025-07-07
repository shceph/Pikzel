#pragma once

#include "camera.hpp"
#include "gla/pixel_buffer.hpp"
#include "project.hpp"
#include "selection.hpp"
#include "tool.hpp"

#include "gla/texture.hpp"

#include <imgui.h>

#include <optional>
#include <string>

namespace Pikzel
{
struct Color
{
    auto operator=(const ImVec4& color) -> Color&;
    auto operator==(const Color& other) const -> bool;
    auto operator==(const ImVec4& other) const -> bool;

    static auto BlendColor(Color color1, Color color2) -> Color;
    static auto FromImVec4(ImVec4 color) -> Color;
    static auto FromGlaColor(Gla::Color color) -> Color;

    uint8_t r = 0, g = 0, b = 0, a = 0;
};

static constexpr Color kColorTransparent{.r = 0, .g = 0, .b = 0, .a = 0};
static constexpr Color kColorSelectionPreview{
    .r = 45, .g = 50, .b = 220, .a = 100};

struct Vertex
{
    float pos_x{}, pos_y{};
    Color color{.r = 0, .g = 0, .b = 0, .a = 0};
};

class Layer
{
  public:
    struct RectShapeData
    {
        bool shape_began = false;
        Vec2Int shape_begin_coords{0, 0};
    };

    explicit Layer(Tool& tool, Camera& camera, Selection& selection,
                   Gla::PboMappedBuffSpan& pbo_buff, Vec2Int canvas_dims,
                   bool is_canvas_layer = true,
                   bool draw_visible_pixels_only = false) noexcept;

    using ShouldUpdateHistory = bool;
    auto DoCurrentTool() -> ShouldUpdateHistory;
    void Update();

    void SwitchVisibilityState() { mVisible = !mVisible; }
    void SwitchLockState() { mLocked = !mLocked; }

    [[nodiscard]] auto IsEdited() const -> bool { return mIsEdited; }
    [[nodiscard]] auto IsVisible() const -> bool { return mVisible; }
    [[nodiscard]] auto IsLocked() const -> bool { return mLocked; }
    [[nodiscard]] auto GetOpacity() const -> int { return mOpacity; }
    [[nodiscard]] auto GetName() const -> const std::string&
    {
        return mLayerName;
    }
    [[nodiscard]] auto GetPixel(Vec2Int coords) const -> Color
    {
        auto col = mPboBuff.get()[(coords.y * mCanvasDims.x) + coords.x];
        return {.r = col.r, .g = col.g, .b = col.b, .a = col.a};
    }
    [[nodiscard]] auto GetCanvasDims() const -> Vec2Int { return mCanvasDims; }
    [[nodiscard]] auto IsPreviewLayer() const -> bool
    {
        return !mIsCanvasLayer;
    }
    [[nodiscard]] auto GetTexture() const -> const Gla::Texture2D&
    {
        return mTex;
    }
    [[nodiscard]] auto GetTexture() -> Gla::Texture2D& { return mTex; }

    // Returns Vec2Int if the cursor is above canvas, otherwise returns
    // std::nullopt
    [[nodiscard]]
    auto CanvasCoordsFromCursorPos() const -> std::optional<Vec2Int>;
    auto ClampToCanvasDims(Vec2Int val_to_clamp) -> Vec2Int;

    static void ResetConstructCounter() { sConstructCounter = 1; }

    // Custom delete color can be set, I'm using this for the preview layer
    // where I want the brush to have a specific color when I'm using eraser.
    void DrawCircle(Vec2Int center, int radius, bool fill,
                    Color delete_color = {.r = 0, .g = 0, .b = 0, .a = 0},
                    std::optional<Color> draw_color = std::nullopt);
    void Clear();

  private:
    auto HandleBrushAndEraser() -> ShouldUpdateHistory;
    void HandleColorPicker();
    auto HandleBucket() -> ShouldUpdateHistory;
    auto HandleRectShape() -> ShouldUpdateHistory;
    void DrawPixel(Vec2Int coords);
    void DrawPixel(Vec2Int coords, Color color);
    void DrawPixelClampCoords(Vec2Int coords, Color color);
    void DrawRect(Vec2Int upper_left, Vec2Int bottom_right, bool fill,
                  std::optional<Color> color = std::nullopt);
    void DrawThickLine(Vec2Int point_a, Vec2Int point_b, int thickness,
                       Color color);
    void DrawLine(Vec2Int point_a, Vec2Int point_b, int thickness,
                  std::optional<Color> color = std::nullopt);
    void DrawLine(Vec2Int point_a, Vec2Int point_b,
                  std::optional<Color> color = std::nullopt);
    void Fill(int x_coord, int y_coord, Color clicked_color);
    void Fill(int x_coord, int y_coord, Color clicked_color, Color fill_color);
    void FillUntil(Color until_color, int x_coord, int y_coord,
                   Color fill_color);

    RectShapeData mHandleRectShapeData;
    Vec2Int mCanvasDims;
    bool mIsCanvasLayer;
    bool mIsEdited{false};
    bool mVisible{true};
    bool mLocked{false};
    bool mDrawVisiblePixelsOnly{false};
    int mOpacity{255};
    std::string mLayerName;
    std::reference_wrapper<Tool> mTool;
    std::reference_wrapper<Camera> mCamera;
    std::reference_wrapper<Selection> mSelection;
    std::reference_wrapper<Gla::PboMappedBuffSpan> mPboBuff;
    Gla::Texture2D mTex;

    inline static int sConstructCounter{1};

    friend class UI;
    friend class LayerControl;
    friend class PreviewLayer;
    friend void Project::Open(const std::string&);
};
} // namespace Pikzel
