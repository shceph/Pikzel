#pragma once

#include "camera.hpp"
#include "layer.hpp"
#include "preview_layer.hpp"
#include "selection.hpp"
#include "tool.hpp"
#include "tree.hpp"

#include <cstddef>
#include <cassert>
#include <list>
#include <optional>
#include <vector>
#include <utility>

#include "gla/texture.hpp"
#include "gla/pixel_buffer.hpp"

#include <GLFW/glfw3.h>

namespace Pikzel {
class LayerControl {
  public:
    struct Capture {
        Capture(Tool& tool, Camera& camera, Selection& selection,
                Gla::PboMappedBuffSpan& pbo_buff, Vec2 canvas_dims,
                std::size_t selected_layer_ind)
            : time_of_creation{static_cast<int>(glfwGetTime())},
              selected_layer_index{selected_layer_ind} {
            layers.emplace_back(tool, camera, selection, pbo_buff, canvas_dims);
        }

        Capture(std::list<Layer>& layers, std::size_t selected_layer_index)
            : time_of_creation{static_cast<int>(glfwGetTime())}, layers{layers},
              selected_layer_index{selected_layer_index} {}

        int time_of_creation;
        std::list<Layer> layers;
        std::size_t selected_layer_index;
    };

    [[nodiscard]] auto GetCurrentLayer() -> Layer&;
    [[nodiscard]] auto GetCurrentLayer() const -> const Layer&;
    void SetCurrentLayer(std::size_t layer_index);
    [[nodiscard]]
    auto GetCanvasDims() const -> Vec2;
    auto HandleRectShape(PreviewLayer& preview_layer, Color tool_color,
                         Layer::CanvasWindowData win_data) const
        -> std::optional<std::pair<Vec2, Vec2>>;
    auto HandleSelectionTool(PreviewLayer& preview_layer,
                             Layer::CanvasWindowData win_data) const
        -> std::optional<std::pair<Vec2, Vec2>>;
    void HandleColorSelectionTool(PreviewLayer& preview_layer_for_selection,
                                  int color_selection_threshold,
                                  Layer::CanvasWindowData win_data);
    void HandleMoveSelectionTool(PreviewLayer& preview_layer,
                                 PreviewLayer& preview_layer_for_selection,
                                 Layer::CanvasWindowData win_data);
    void SelectByColor(Color col, int threshold);
    void DoCurrentTool(PreviewLayer& preview_layer, Tool& tool,
                       PreviewLayer& preview_layer_for_selection,
                       Layer::CanvasWindowData win_data,
                       int color_selection_threshold = 0);
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
                       PreviewLayer& preview_layer,
                       PreviewLayer& preview_layer_for_selection,
                       Gla::PixelBuffer& pbo, Layer::CanvasWindowData win_data,
                       int color_selection_threshold);
    void InitHistory(Camera& camera, Tool& tool);
    void WriteCurrentLayerTextureDataToPbo();
    void UpdateMappedPBOMemorySpanForAllLayers(Gla::PboMappedBuffSpan pbo_buff);
    void MoveSelectedPixelsInCurrentLayer(Vec2 offset);
    void UpdatePreviewLayerForSelection(
        PreviewLayer& preview_layer_for_selection) const;
    [[nodiscard]] auto AreCoordsInBounds(Vec2 coords) const -> bool;

    [[nodiscard]] auto GetSelection() const -> const Selection& {
        return mSelection;
    }

    [[nodiscard]] auto GetSelection() -> Selection& { return mSelection; }
    [[nodiscard]] auto GetLayerCount() const -> std::size_t {
        assert(!GetLayers().empty());
        return GetLayers().size();
    }
    [[nodiscard]] auto GetLayers() const -> const std::list<Layer>& {
        assert(mCurrentCapture.has_value());
        return mCurrentCapture->layers;
    }
    [[nodiscard]] auto GetLayers() -> std::list<Layer>& {
        assert(mCurrentCapture.has_value());
        return mCurrentCapture->layers;
    }
    [[nodiscard]] auto
    CanvasCoordsFromCursorPos(Layer::CanvasWindowData win_data) const
        -> std::optional<Vec2> {
        return GetLayers().cbegin()->CanvasCoordsFromCursorPos(win_data);
    }
    [[nodiscard]] auto GetCurrentLayerIndex() const -> std::size_t {
        return mCurrentLayerIndex;
    }
    [[nodiscard]] auto GetUndoTree() const -> const Tree<Capture>& {
        assert(mUndoTree.has_value());
        return *mUndoTree;
    }
    [[nodiscard]] auto GetUndoTree() -> Tree<Capture>& {
        assert(mUndoTree.has_value());
        return *mUndoTree;
    }
    [[nodiscard]] auto GetCurrentUndoTreeNode() const -> const Tree<Capture>& {
        assert(mCurrentUndoTreeNode != nullptr);
        return *mCurrentUndoTreeNode;
    }
    [[nodiscard]] auto GetCurrentLayerTexture() const -> const Gla::Texture2D& {
        return GetCurrentLayer().GetTexture();
    }
    [[nodiscard]] auto GetCurrentLayerTexture() -> Gla::Texture2D& {
        return GetCurrentLayer().GetTexture();
    }
    [[nodiscard]] auto GetCurrentToolType() const -> ToolType {
        return GetCurrentLayer().mTool.get().GetToolType();
    }
    [[nodiscard]] auto HaveChosenDifferentLayerThisFrame() const -> bool {
        return mCurrentLayerIndex != mCurrentLayerIndexTemp;
    }
    [[nodiscard]] auto HasToolTypeChanged() const -> bool {
        return GetCurrentToolType() != mPreviousToolType;
    }

    void SetCurrentLayerIndex(std::size_t idx) { mCurrentLayerIndex = idx; }
    void SetCanvasDims(Vec2 canvas_dims) { mCanvasDims = canvas_dims; }
    void MarkForUndo() { mShouldUndo = true; }
    void MarkForRedo() { mShouldRedo = true; }
    void MarkToAddLayer() { mShouldAddLayer = true; }

  private:
    auto GetCapture() -> Capture& {
        assert(mCurrentCapture.has_value());
        return *mCurrentCapture;
    }
    void MarkHistoryForUpdate() { mShouldUpdateHistory = true; }

    static constexpr int kMaxHistoryLenght = 30;

    Selection mSelection;
    Tree<Capture>* mCurrentUndoTreeNode{nullptr};
    std::optional<Tree<Capture>> mUndoTree{std::nullopt};
    std::optional<Capture> mCurrentCapture{std::nullopt};
    Gla::PboMappedBuffSpan mPboBuff;
    std::size_t mCurrentLayerIndex{0};
    std::size_t mCurrentLayerIndexTemp{0};
    Vec2 mCanvasDims{0, 0};
    bool mShouldUpdateHistory{false};
    bool mShouldUndo{false};
    bool mShouldRedo{false};
    bool mShouldAddLayer{false};
    // Used for checking if the tool type has changed. It gets updated in
    // Update()
    ToolType mPreviousToolType = ToolType::kBrush;

    friend class Layer;
    friend class Project;
};
} // namespace Pikzel
