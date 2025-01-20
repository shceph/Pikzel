#pragma once

#include "Project.hpp"
#include "Tool.hpp"

#include <imgui.h>

#include <deque>
#include <list>
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
    Color color{0, 0, 0, 0};
};

using CanvasData = std::vector<std::vector<Color>>;

class Layer
{
  public:
	struct RectShapeData
	{
		bool shape_began = false;
		Vec2Int shape_begin_coords{0, 0};
	};

    explicit Layer(bool is_canvas_layer = true) noexcept;
    void DoCurrentTool();
    void EmplaceVertices(std::vector<Vertex>& vertices,
                         bool use_color_alpha = false) const;
    void Update();

    inline void SwitchVisibilityState() { mVisible = !mVisible; }
    inline void SwitchLockState() { mLocked = !mLocked; }

    [[nodiscard]] inline auto IsVisible() const -> bool { return mVisible; }
    [[nodiscard]] inline auto IsLocked() const -> bool { return mLocked; }
    [[nodiscard]] inline auto GetOpacity() const -> int { return mOpacity; }
    [[nodiscard]] inline auto GetName() const -> const std::string&
    {
        return mLayerName;
    }
    [[nodiscard]] inline auto GetPixel(Vec2Int coords) const -> Color
    {
        return mCanvas[coords.y][coords.x];
    }

    // Returns Vec2Int if the cursor is above canvas, otherwise returns
    // std::nullopt
    static auto CanvasCoordsFromCursorPos() -> std::optional<Vec2Int>;
    static auto ClampToCanvasDims(Vec2Int val_to_clamp) -> Vec2Int;

    static inline void ResetConstructCounter() { sConstructCounter = 1; }
    void DrawCircle(Vec2Int center, int radius, bool fill,
                    Color delete_color = {0, 0, 0, 0});
    void Clear();

    void HandleRectShape();

  private:
    void HandleBrushAndEraser();
    static void HandleColorPicker();
    void HandleBucket();
    void DrawPixel(Vec2Int coords,
                   Color color = Color::FromImVec4(Tool::GetColor()));
    void DrawRect(Vec2Int upper_left, Vec2Int bottom_right, bool fill);
    void DrawLine(Vec2Int point_a, Vec2Int point_b, int thickness);
    void DrawLine(Vec2Int point_a, Vec2Int point_b);
    void Fill(int x_coord, int y_coord, Color clicked_color);

    CanvasData mCanvas;
	RectShapeData mRectShapeData;
    bool mIsCanvasLayer;
    bool mVisible = true;
    bool mLocked = false;
    int mOpacity = 255;
    std::string mLayerName;

    inline static int sConstructCounter = 1;

    friend class UI;
    friend class Layers;
    friend void Project::SaveAsProject(const std::string&);
    friend void Project::Open(const std::string&);
};

class Layers
{
  public:
    struct Capture
    {
        Capture() : selected_layer_index{sCurrentLayerIndex}
        {
            layers.emplace_back();
        }

        Capture(std::list<Layer>& layers, std::size_t selected_layer_index)
            : layers{layers}, selected_layer_index{selected_layer_index}
        {
        }

        std::list<Layer> layers;
        std::size_t selected_layer_index;
    };

    static auto GetCurrentLayer() -> Layer&;
    static void DoCurrentTool();
    static void MoveUp(std::size_t layer_index);
    static void MoveDown(std::size_t layer_index);
    static void AddLayer();
    static void EmplaceVertices(std::vector<Vertex>& vertices);
    static void EmplaceBckgVertices(std::vector<Vertex>& vertices);
    static void ResetDataToDefault();
    static void DrawToTempLayer();
    static auto AtIndex(std::size_t index) -> Layer&;
    static auto GetDisplayedCanvas() -> CanvasData;
    static void PushToHistory();
    static void Undo();
    static void Redo();
    static void Update();

    inline static auto GetLayerCount() -> std::size_t
    {
        assert(!GetLayers().empty());
        return GetLayers().size();
    }
    inline static auto GetLayersConst() -> const std::list<Layer>&
    {
        return GetLayers();
    }
    inline static auto GetCurrentLayerIndex() -> std::size_t
    {
        return sCurrentLayerIndex;
    }
    inline static void InitHistory()
    {
        auto& history = GetHistory();
        history.clear();
        history.emplace_back();
        sCurrentCapture = 0;
    }

  private:
    inline static auto GetCapture() -> Capture&
    {
        auto& history = GetHistory();
        assert(sCurrentCapture < history.size());
        return history[sCurrentCapture];
    }
    inline static auto GetLayers() -> std::list<Layer>&
    {
        return GetCapture().layers;
    }
    inline static auto GetHistory() -> std::deque<Capture>&
    {
        static std::deque<Capture> history;
        return history;
    }
    inline static void MarkHistoryForUpdate() { sShouldUpdateHistory = true; }

    static constexpr int kMaxHistoryLenght = 30;
    inline static std::size_t sCurrentCapture = 0;
    inline static std::size_t sCurrentLayerIndex = 0;
    inline static bool sShouldUpdateHistory = false;

    friend class Layer;
    friend class UI;
    friend class VertexBufferControl;
    friend void Project::New(Vec2Int);
    friend void Project::Open(const std::string&);
    friend void Project::SaveAsProject(const std::string&);
};
} // namespace Pikzel
