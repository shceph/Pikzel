#pragma once

#include "camera.hpp"
#include "layer.hpp"
#include "project.hpp"
#include "tool.hpp"
#include "tree.hpp"

#include <imgui.h>

#include <list>
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
        Capture(Tool& tool, Camera& camera, Vec2Int canvas_dims,
                std::size_t selected_layer_ind)
            : selected_layer_index{selected_layer_ind}
        {
            layers.emplace_back(tool, camera, canvas_dims);
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
    void AddLayer(Tool& tool, Camera& camera);
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
    void SetCurrentNode(Tree<Capture>& node_to_set_to);
    void UpdateAndDraw(bool should_do_tool, Tool& tool, Camera& camera);
    void InitHistory(Camera& camera, Tool& tool);

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
    [[nodiscard]] auto GetUndoTree() const -> const Tree<Capture>&
    {
        assert(mUndoTree.has_value());
        return *mUndoTree;
    }
    [[nodiscard]] auto GetUndoTree() -> Tree<Capture>&
    {
        assert(mUndoTree.has_value());
        return *mUndoTree;
    }
    [[nodiscard]] auto GetCurrentUndoTreeNode() const -> const Tree<Capture>&
    {
        assert(mCurrentUndoTreeNode != nullptr);
        return *mCurrentUndoTreeNode;
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
