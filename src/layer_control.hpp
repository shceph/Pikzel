#pragma once

#include "camera.hpp"
#include "layer.hpp"
#include "project.hpp"
#include "tool.hpp"
#include "tree.hpp"

#include <imgui.h>

#include <list>
#include <memory>
#include <optional>
#include <string>
#include <vector>

namespace Pikzel
{
class Layers
{
  public:
    struct Capture
    {
        Capture(std::shared_ptr<Tool> tool, std::shared_ptr<Camera> camera,
                Vec2Int canvas_dims, std::size_t selected_layer_ind)
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
    [[nodiscard]]
    auto GetDisplayedCanvas() const -> CanvasData;
    void PushToHistory();
    void Undo();
    void Redo();
    void UpdateAndDraw(bool should_do_tool, std::shared_ptr<Tool> tool,
                       std::shared_ptr<Camera> camera);
    void InitHistory(std::shared_ptr<Camera> camera,
                     std::shared_ptr<Tool> tool);

    [[nodiscard]] auto GetLayerCount() const -> std::size_t
    {
        assert(!GetLayers().empty());
        return GetLayers().size();
    }
    [[nodiscard]] auto GetLayers() const -> const std::list<Layer>&
    {
        assert(mCurrentCapture.has_value());
        return mCurrentCapture->layers;
    }
    [[nodiscard]] auto CanvasCoordsFromCursorPos() const
        -> std::optional<Vec2Int>
    {
        return GetLayers().cbegin()->CanvasCoordsFromCursorPos();
    }
    [[nodiscard]] auto GetCurrentLayerIndex() const -> std::size_t
    {
        return mCurrentLayerIndex;
    }
    void SetCanvasDims(Vec2Int canvas_dims) { mCanvasDims = canvas_dims; }
    void MarkForUndo() { mShouldUndo = true; }
    void MarkForRedo() { mShouldRedo = true; }
    void MarkToAddLayer() { mShouldAddLayer = true; }

  private:
    auto GetCapture() -> Capture&
    {
        assert(mCurrentCapture.has_value());
        return *mCurrentCapture;
    }
    auto GetLayers() -> std::list<Layer>& { return GetCapture().layers; }
    void MarkHistoryForUpdate() { mShouldUpdateHistory = true; }

    static constexpr int kMaxHistoryLenght = 30;

    Tree<Capture>* mCurrentUndoTreeNode{nullptr};
    std::optional<Tree<Capture>> mUndoTree{std::nullopt};
    std::optional<Capture> mCurrentCapture{std::nullopt};
    std::size_t mCurrentLayerIndex{0};
    Vec2Int mCanvasDims{0, 0};
    bool mShouldUpdateHistory{false};
    bool mShouldUndo{false};
    bool mShouldRedo{false};
    bool mShouldAddLayer{false};

    friend class Layer;
    friend class UI;
    friend class VertexBufferControl;
    friend void Project::New(Vec2Int);
    friend void Project::Open(const std::string&);
    friend void Project::SaveAsProject(const std::string&);
};
} // namespace Pikzel
