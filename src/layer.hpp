#pragma once

#include "camera.hpp"
#include "project.hpp"
#include "tool.hpp"

#include <imgui.h>

#include <deque>
#include <list>
#include <memory>
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

    explicit Layer(std::shared_ptr<Tool> tool, std::shared_ptr<Camera> camera,
                   Vec2Int canvas_dims, bool is_canvas_layer = true) noexcept;

    using ShouldUpdateHistory = bool;
    auto DoCurrentTool() -> ShouldUpdateHistory;
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
        std::lock_guard<std::mutex> lock{sMutex};
        return mCanvas[coords.y][coords.x];
    }
    [[nodiscard]] inline auto GetCanvas() const -> const CanvasData&
    {
        return mCanvas;
    }
    [[nodiscard]] inline auto GetCanvasDims() const -> Vec2Int
    {
        return mCanvasDims;
    }
    [[nodiscard]] inline auto IsPreviewLayer() const -> bool
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
    static inline auto ShouldUpdateWholeVBO() -> bool
    {
        return sShouldUpdateWholeVBO;
    }
    static inline auto GetDirtyPixels() -> std::vector<Vec2Int>&
    {
        static std::vector<Vec2Int> dirty_pixels;
        return dirty_pixels;
    }

    static inline void ResetConstructCounter() { sConstructCounter = 1; }
    // Custom delete color can be set, I'm using this for the preview layer
    // where I want the brush to have a specific color.
    void DrawCircle(Vec2Int center, int radius, bool fill,
                    Color delete_color = {0, 0, 0, 0});
    void Clear();

  private:
    auto HandleBrushAndEraser() -> ShouldUpdateHistory;
    void HandleColorPicker();
    auto HandleBucket() -> ShouldUpdateHistory;
    auto HandleRectShape() -> ShouldUpdateHistory;
    void DrawPixel(Vec2Int coords);
    void DrawPixel(Vec2Int coords, Color color);
    void DrawRect(Vec2Int upper_left, Vec2Int bottom_right, bool fill);
    void DrawLine(Vec2Int point_a, Vec2Int point_b, int thickness,
                  std::optional<Color> color = std::nullopt);
    void DrawLine(Vec2Int point_a, Vec2Int point_b);
    void Fill(int x_coord, int y_coord, Color clicked_color);

    CanvasData mCanvas;
    RectShapeData mHandleRectShapeData;
    Vec2Int mCanvasDims;
    bool mIsCanvasLayer;
    bool mVisible = true;
    bool mLocked = false;
    int mOpacity = 255;
    std::string mLayerName;
    std::shared_ptr<Tool> mTool;
    std::shared_ptr<Camera> mCamera;

    inline static std::mutex sMutex;
    inline static int sConstructCounter = 1;
    inline static bool sShouldUpdateWholeVBO = true;

    friend class UI;
    friend class Layers;
    friend class PreviewLayer;
    friend void Project::Open(const std::string&);
};

class Layers
{
  public:
    struct Capture
    {
        explicit Capture(std::shared_ptr<Tool> tool,
                         std::shared_ptr<Camera> camera, Vec2Int canvas_dims,
                         std::size_t selected_layer_ind)
            : selected_layer_index{selected_layer_ind}
        {
            layers.emplace_back(std::move(tool), std::move(camera),
                                canvas_dims);
        }

        Capture(std::list<Layer>& layers, std::size_t selected_layer_index)
            : layers{layers}, selected_layer_index{selected_layer_index}
        {
        }

        std::list<Layer> layers;
        std::size_t selected_layer_index;
    };

    auto GetCurrentLayer() -> Layer&;
    [[nodiscard]]
    auto GetCanvasDims() const -> Vec2Int;
    void DoCurrentTool();
    void MoveUp(std::size_t layer_index);
    void MoveDown(std::size_t layer_index);
    void AddLayer(std::shared_ptr<Tool> tool, std::shared_ptr<Camera> camera);
    void EmplaceVertices(std::vector<Vertex>& vertices) const;
    void EmplaceBckgVertices(std::vector<Vertex>& vertices,
                             std::optional<Vec2Int> custom_dims) const;
    void ResetDataToDefault();
    void DrawToTempLayer();
    auto AtIndex(std::size_t index) -> Layer&;
    auto GetDisplayedCanvas() -> CanvasData;
    void PushToHistory();
    void Undo();
    void Redo();
    void UpdateAndDraw(bool should_do_tool, std::shared_ptr<Tool> tool,
                       std::shared_ptr<Camera> camera);

    [[nodiscard]] inline auto GetLayerCount() const -> std::size_t
    {
        assert(!GetLayers().empty());
        return GetLayers().size();
    }
    [[nodiscard]] inline auto GetLayers() const -> const std::list<Layer>&
    {
        return mHistory[mCurrentCapture].layers;
    }
    [[nodiscard]] inline auto
    CanvasCoordsFromCursorPos() const -> std::optional<Vec2Int>
    {
        return GetLayers().cbegin()->CanvasCoordsFromCursorPos();
    }
    [[nodiscard]] inline auto GetCurrentLayerIndex() const -> std::size_t
    {
        return mCurrentLayerIndex;
    }
    inline void SetCanvasDims(Vec2Int canvas_dims)
    {
        mCanvasDims = canvas_dims;
    }
    inline void InitHistory(std::shared_ptr<Camera> camera,
                            std::shared_ptr<Tool> tool)
    {
        mHistory.clear();
        mHistory.emplace_back(std::move(tool), std::move(camera), mCanvasDims,
                              0);
        mCurrentCapture = 0;
    }
    inline void MarkForUndo() { mShouldUndo = true; }
    inline void MarkForRedo() { mShouldRedo = true; }
    inline void MarkToAddLayer() { mShouldAddLayer = true; }

  private:
    inline auto GetCapture() -> Capture&
    {
        assert(mCurrentCapture < mHistory.size());
        return mHistory[mCurrentCapture];
    }
    inline auto GetLayers() -> std::list<Layer>& { return GetCapture().layers; }
    /* inline auto GetHistory() -> std::deque<Capture>& { return mHistory; } */
    inline void MarkHistoryForUpdate() { mShouldUpdateHistory = true; }

    static constexpr int kMaxHistoryLenght = 30;

    std::deque<Capture> mHistory;
    std::size_t mCurrentCapture = 0;
    std::size_t mCurrentLayerIndex = 0;
    Vec2Int mCanvasDims{0, 0};
    bool mShouldUpdateHistory = false;
    bool mShouldUndo = false;
    bool mShouldRedo = false;
    bool mShouldAddLayer = false;

    friend class Layer;
    friend class UI;
    friend class VertexBufferControl;
    friend void Project::New(Vec2Int);
    friend void Project::Open(const std::string&);
    friend void Project::SaveAsProject(const std::string&);
};
} // namespace Pikzel
