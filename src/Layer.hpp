#pragma once

#include "Project.hpp"

#include <imgui.h>
#include <list>
#include <string>
#include <vector>

namespace App
{
constexpr int kCanvasHeight = 32;
constexpr int kCanvasWidth = 32;

struct Color
{
    uint8_t r = 0, g = 0, b = 0, a = 0;

    auto operator=(const ImVec4& color) -> Color&;
    auto operator==(const Color& other) const -> bool;
    auto operator==(const ImVec4& other) const -> bool;

    static auto BlendColor(Color color1, Color color2) -> Color;
    static auto FromImVec4(ImVec4 color) -> Color;
};

struct Vertex
{
    float pos_x{}, pos_y{};
    Color color;
};

using CanvasData = std::vector<std::vector<Color>>;

class Layer
{
  public:
    Layer() noexcept;
    void DoCurrentTool();
    // Places the vertices containing the canvas data for my shader program
    void EmplaceVertices(std::vector<Vertex>& vertices,
                         bool use_color_alpha = false);

    [[nodiscard]] inline auto IsVisible() const -> bool { return mVisible; }
    [[nodiscard]] inline auto IsLocked() const -> bool { return mLocked; }

    inline void SwitchVisibilityState() { mVisible = !mVisible; }
    inline void SwitchLockState() { mLocked = !mLocked; }

    [[nodiscard]] inline auto GetName() const -> const std::string&
    {
        return mLayerName;
    }

    static auto CanvasCoordsFromCursorPos(Vec2Int& coords) -> bool;

  private:
    void DrawCircle(Vec2Int center, int radius, bool only_outline,
                    Color delete_color = {0, 0, 0, 0});
    void DrawLine(Vec2Int point_a, Vec2Int point_b, int thickness);
    void DrawLine(Vec2Int point_a, Vec2Int point_b);
    void Fill(int x_coord, int y_coord, Color clicked_color);

    CanvasData mCanvas;

    bool mVisible = true, mLocked = false;
    int mOpacity = 255;

    std::string mLayerName;

    friend class UI;
    friend class Layers;
};

class Layers
{
  public:
    static auto GetCurrentLayer() -> Layer&;

    static void DoCurrentTool();
    static void MoveUp(std::size_t layer_index);
    static void MoveDown(std::size_t layer_index);
    static void AddLayer();
    static void EmplaceVertices(std::vector<Vertex>& vertices);
    static void ResetDataToDefault();
    static void DrawToTempLayer();

    static auto AtIndex(std::size_t index) -> Layer&;

    static auto GetDisplayedCanvas() -> CanvasData;
    inline static auto GetLayerCount() -> std::size_t
    {
        return GetLayers().size();
    }

  private:
    static auto GetLayers() -> std::list<Layer>&
    {
	   static std::list<Layer> layers;
	   return layers;
    }
    static auto GetTempLayer() -> Layer&
    {
        static Layer tmp_lay;
        return tmp_lay;
    }
    inline static std::size_t sCurrentLayerIndex = 0;

    friend class UI;
    friend class Layer;
};
} // namespace App
