#pragma once

#include "camera.hpp"
#include "layer.hpp"
#include "preview_layer.hpp"
#include "project.hpp"
#include "selection.hpp"
#include "tool.hpp"
#include "tree.hpp"

#include "gla/texture.hpp"

#include <GLFW/glfw3.h>
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
        Capture(Tool& tool, Camera& camera, Selection& selection,
                Gla::PboMappedBuffSpan& pbo_buff, Vec2Int canvas_dims,
                std::size_t selected_layer_ind)
            : time_of_creation{static_cast<int>(glfwGetTime())},
              selected_layer_index{selected_layer_ind}
        {
            layers.emplace_back(tool, camera, selection, pbo_buff, canvas_dims);
        }

        Capture(std::list<Layer>& layers, std::size_t selected_layer_index)
            : time_of_creation{static_cast<int>(glfwGetTime())}, layers{layers},
              selected_layer_index{selected_layer_index}
        {
        }

        int time_of_creation;
        std::list<Layer> layers;
        std::size_t selected_layer_index;
    };

    explicit Layers(Gla::PboMappedBuffSpan& pbo_buff);

    [[nodiscard]] auto GetCurrentLayer() -> Layer&;
    [[nodiscard]] auto GetCurrentLayer() const -> const Layer&;
    [[nodiscard]]
    auto GetCanvasDims() const -> Vec2Int;
    auto HandleSelectionTool(PreviewLayer& preview_layer) const
        -> std::optional<std::pair<Vec2Int, Vec2Int>>;
    auto HandleRectShape(PreviewLayer& preview_layer, Color tool_color) const
        -> std::optional<std::pair<Vec2Int, Vec2Int>>;
    void DoCurrentTool(Tool& tool, PreviewLayer& preview_layer);
    void MoveUp(std::size_t layer_index);
    void MoveDown(std::size_t layer_index);
    void AddLayer(Tool& tool, Camera& camera);
    void ResetDataToDefault();
    void DrawToTempLayer();
    auto AtIndex(std::size_t index) -> Layer&;
    [[nodiscard]]
    auto GetDisplayedCanvas() const -> std::vector<Color>;
    void PushToHistory();
    void Undo();
    void Redo();
    void SetCurrentNode(Tree<Capture>& node_to_set_to);
    void UpdateAndDraw(bool should_do_tool, Tool& tool, Camera& camera,
                       PreviewLayer& preview_layer);
    void InitHistory(Camera& camera, Tool& tool);
    void WriteCurrentLayerTextureDataToPbo();

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
    [[nodiscard]] auto GetLayers() -> std::list<Layer>&
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
    [[nodiscard]] auto GetCurrentLayerTexture() const -> const Gla::Texture2D&
    {
        return GetCurrentLayer().GetTexture();
    }
    auto GetCurrentLayerTexture() -> Gla::Texture2D&
    {
        return GetCurrentLayer().GetTexture();
    }
    [[nodiscard]] auto HaveChosenDifferentLayerThisFrame() const -> bool
    {
        return mCurrentLayerIndex != mCurrentLayerIndexTemp;
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
    void MarkHistoryForUpdate() { mShouldUpdateHistory = true; }

    static constexpr int kMaxHistoryLenght = 30;

    Selection mSelection;
    Tree<Capture>* mCurrentUndoTreeNode{nullptr};
    std::optional<Tree<Capture>> mUndoTree{std::nullopt};
    std::optional<Capture> mCurrentCapture{std::nullopt};
    std::reference_wrapper<Gla::PboMappedBuffSpan> mPboBuff;
    std::size_t mCurrentLayerIndex{0};
    std::size_t mCurrentLayerIndexTemp{0};
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
