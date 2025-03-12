#pragma once

#include "camera.hpp"
#include "project.hpp"
#include "tool.hpp"

#include <imgui.h>

#include <mutex>
#include <optional>
#include <string>
#include <vector>

namespace Pikzel
{
constexpr int kCanvasHeight = 32;
constexpr int kCanvasWidth = 32;

struct Color
{
    auto operator=(const ImVec4& color) -> Color&;
    auto operator==(const Color& other) const -> bool;
    auto operator==(const ImVec4& other) const -> bool;

    static auto BlendColor(Color color1, Color color2) -> Color;
    static auto FromImVec4(ImVec4 color) -> Color;

    uint8_t r = 0, g = 0, b = 0, a = 0;
};

struct Vertex
{
    float pos_x{}, pos_y{};
    Color color{.r = 0, .g = 0, .b = 0, .a = 0};
};

using CanvasData = std::vector<Color>;

class Layer
{
  public:
    struct RectShapeData
    {
        bool shape_began = false;
        Vec2Int shape_begin_coords{0, 0};
    };

    explicit Layer(Tool& tool, Camera& camera, Vec2Int canvas_dims,
                   bool is_canvas_layer = true,
                   bool draw_visible_pixels_only = false) noexcept;

    using ShouldUpdateHistory = bool;
    auto DoCurrentTool() -> ShouldUpdateHistory;
    void EmplaceVertices(std::vector<Vertex>& vertices,
                         bool use_color_alpha = false) const;
    void Update();

    void SwitchVisibilityState() { mVisible = !mVisible; }
    void SwitchLockState() { mLocked = !mLocked; }

    [[nodiscard]] auto IsVisible() const -> bool { return mVisible; }
    [[nodiscard]] auto IsLocked() const -> bool { return mLocked; }
    [[nodiscard]] auto GetOpacity() const -> int { return mOpacity; }
    [[nodiscard]] auto GetName() const -> const std::string&
    {
        return mLayerName;
    }
    [[nodiscard]] auto GetPixel(Vec2Int coords) const -> Color
    {
        std::lock_guard<std::mutex> lock{sMutex};
        return mCanvas[(coords.y * mCanvasDims.x) + coords.x];
    }
    [[nodiscard]] auto GetCanvas() const -> const CanvasData&
    {
        return mCanvas;
    }
    [[nodiscard]] auto GetCanvasDims() const -> Vec2Int { return mCanvasDims; }
    [[nodiscard]] auto IsPreviewLayer() const -> bool
    {
        return !mIsCanvasLayer;
    }

    // Returns Vec2Int if the cursor is above canvas, otherwise returns
    // std::nullopt
    [[nodiscard]]
    auto CanvasCoordsFromCursorPos() const -> std::optional<Vec2Int>;
    auto ClampToCanvasDims(Vec2Int val_to_clamp) -> Vec2Int;
    static void ResetDirtyPixelData();
    static void SetUpdateWholeVBOToTrue() { sShouldUpdateWholeVBO = true; }
    static auto ShouldUpdateWholeVBO() -> bool { return sShouldUpdateWholeVBO; }
    static auto GetDirtyPixels() -> std::vector<Vec2Int>&
    {
        static std::vector<Vec2Int> dirty_pixels;
        return dirty_pixels;
    }

    static void ResetConstructCounter() { sConstructCounter = 1; }
    // Custom delete color can be set, I'm using this for the preview layer
    // where I want the brush to have a specific color.
    void DrawCircle(Vec2Int center, int radius, bool fill,
                    Color delete_color = {.r = 0, .g = 0, .b = 0, .a = 0});
    void Clear();

  private:
    auto HandleBrushAndEraser() -> ShouldUpdateHistory;
    void HandleColorPicker();
    auto HandleBucket() -> ShouldUpdateHistory;
    auto HandleRectShape() -> ShouldUpdateHistory;
    void DrawPixel(Vec2Int coords);
    void DrawPixel(Vec2Int coords, Color color);
    void DrawPixelClampCoords(Vec2Int coords, Color color);
    void DrawRect(Vec2Int upper_left, Vec2Int bottom_right, bool fill);
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

    CanvasData mCanvas;
    RectShapeData mHandleRectShapeData;
    Vec2Int mCanvasDims;
    bool mIsCanvasLayer;
    bool mVisible = true;
    bool mLocked = false;
    bool mDrawVisiblePixelsOnly = false;
    int mOpacity = 255;
    std::string mLayerName;
    std::reference_wrapper<Tool> mTool;
    std::reference_wrapper<Camera> mCamera;

    inline static std::mutex sMutex;
    inline static int sConstructCounter = 1;
    inline static bool sShouldUpdateWholeVBO = true;

    friend class UI;
    friend class Layers;
    friend class PreviewLayer;
    friend void Project::Open(const std::string&);
};
} // namespace Pikzel
