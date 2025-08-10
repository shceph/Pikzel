#include "layer_control.hpp"
#include "events.hpp"
#include "gla/pixel_buffer.hpp"
#include "layer.hpp"
#include "preview_layer.hpp"
#include "tool.hpp"

#include "GLFW/glfw3.h"
#include <cstddef>
#include <glm/geometric.hpp>

#include <algorithm>
#include <cmath>
#include <list>
#include <ranges>
#include <vector>

namespace Pikzel {
auto LayerControl::GetCurrentLayer() -> Layer& {
    assert(mCurrentLayerIndex < GetLayers().size());

    auto iter = GetLayers().begin();
    std::advance(iter, mCurrentLayerIndex);
    return *iter;
}

auto LayerControl::GetCurrentLayer() const -> const Layer& {
    assert(mCurrentLayerIndex < GetLayers().size());

    auto iter = GetLayers().begin();
    std::advance(iter, mCurrentLayerIndex);
    return *iter;
}

void LayerControl::SetCurrentLayer(std::size_t layer_index) {
    assert(layer_index < GetLayerCount());
    mCurrentLayerIndex = layer_index;
}

auto LayerControl::GetCanvasDims() const -> Vec2 { return mCanvasDims; }

auto LayerControl::HandleRectShape(PreviewLayer& preview_layer,
                                   Color tool_color) const
    -> std::optional<std::pair<Vec2, Vec2>> {
    auto canv_coord = CanvasCoordsFromCursorPos();
    if (!canv_coord.has_value()) {
        return std::nullopt;
    }
    bool left_button_pressed =
        Events::IsMouseButtonPressed(Events::MouseButtons::kButtonLeft);

    static bool shape_began = false;
    static Vec2 shape_begin_coords{0, 0};

    if (!shape_began) {
        if (left_button_pressed) {
            shape_begin_coords = *canv_coord;
            shape_began = true;
        }
        return std::nullopt;
    }

    // Use left shift to force drawing a square
    if (Events::IsKeyboardKeyPressed(GLFW_KEY_LEFT_SHIFT)) {
        auto diff = (*canv_coord) - shape_begin_coords;

        if (std::abs(diff.x) < std::abs(diff.y)) {
            canv_coord->y =
                shape_begin_coords.y + std::abs(diff.x) * glm::sign(diff.y);
        } else {
            canv_coord->x =
                shape_begin_coords.x + std::abs(diff.y) * glm::sign(diff.x);
        }
    }

    if (left_button_pressed) {
        preview_layer.Clear();
        preview_layer.DrawRect(shape_begin_coords, *canv_coord, tool_color);

        return std::nullopt;
    }

    std::pair<Vec2, Vec2> ret{shape_begin_coords, *canv_coord};
    shape_began = false;
    shape_begin_coords = {0, 0};
    return ret;
}

auto LayerControl::HandleSelectionTool(PreviewLayer& preview_layer) const
    -> std::optional<std::pair<Vec2, Vec2>> {
    auto canv_coord = CanvasCoordsFromCursorPos();
    if (!canv_coord.has_value()) {
        return std::nullopt;
    }
    bool left_button_pressed =
        Events::IsMouseButtonPressed(Events::MouseButtons::kButtonLeft);

    static bool shape_began = false;
    static Vec2 shape_begin_coords{0, 0};

    if (!shape_began) {
        if (left_button_pressed) {
            shape_begin_coords = *canv_coord;
            shape_began = true;
        }
        return std::nullopt;
    }

    // Use left shift to force drawing a square
    if (Events::IsKeyboardKeyPressed(GLFW_KEY_LEFT_SHIFT)) {
        auto diff = (*canv_coord) - shape_begin_coords;

        if (std::abs(diff.x) < std::abs(diff.y)) {
            canv_coord->y =
                shape_begin_coords.y + std::abs(diff.x) * glm::sign(diff.y);
        } else {
            canv_coord->x =
                shape_begin_coords.x + std::abs(diff.y) * glm::sign(diff.x);
        }
    }

    if (left_button_pressed) {
        preview_layer.Clear();
        preview_layer.DrawRect(shape_begin_coords, *canv_coord,
                               kColorSelectionPreview);

        return std::nullopt;
    }

    std::pair<Vec2, Vec2> ret{shape_begin_coords, *canv_coord};
    shape_began = false;
    shape_begin_coords = {0, 0};
    return ret;
}

void LayerControl::HandleColorSelectionTool(
    PreviewLayer& preview_layer_for_selection, int threshold) {

    if (!Events::IsMouseButtonPressed(Events::MouseButtons::kButtonLeft)) {
        return;
    }

    std::optional<Vec2> canv_coord = CanvasCoordsFromCursorPos();

    if (!canv_coord.has_value()) {
        return;
    }

    mSelection.Clear();
    mSelection.SetShouldCheckForSelectionValue(true);

    const Layer& curr_lay = GetCurrentLayer();
    Color clicked_color = curr_lay.GetPixel(*canv_coord);

    SelectByColor(clicked_color, threshold);
	UpdatePreviewLayerForSelection(preview_layer_for_selection);
}

void LayerControl::HandleMoveSelectionTool(
    PreviewLayer& preview_layer, PreviewLayer& preview_layer_for_selection) {
    static bool should_update_prev_lay_after_moving_selection = false;

    if (HasToolTypeChanged() || should_update_prev_lay_after_moving_selection) {
        should_update_prev_lay_after_moving_selection = false;

        const std::vector<bool>& selected_pixels =
            mSelection.GetSelectedPixels();

        UpdatePreviewLayerForSelection(preview_layer_for_selection);

        preview_layer.Clear();

        for (std::size_t i = 0;
             i < static_cast<std::size_t>(mCanvasDims.x) * mCanvasDims.y; i++) {
            if (selected_pixels[i]) {
                preview_layer.DrawPixel(
                    i, Color{.r = 134, .g = 13, .b = 34, .a = 128});
            }
        }

        return;
    }

    auto canv_coord = CanvasCoordsFromCursorPos();

    if (Events::IsMouseButtonPressedDelayed(Events::MouseButtons::kButtonLeft,
                                            std::chrono::milliseconds{100}) &&
        canv_coord.has_value()) {
        should_update_prev_lay_after_moving_selection = true;
        Vec2 center = GetCanvasDims() / 2;
        Vec2 offset = *canv_coord - center;
        MoveSelectedPixelsInCurrentLayer(offset);
        MarkHistoryForUpdate();
    }
}

void LayerControl::SelectByColor(Color col, int threshold) {
    const Layer& curr_lay = GetCurrentLayer();
    std::vector<bool>& selected_pixels = mSelection.GetSelectedPixels();

    for (std::size_t i = 0;
         i < static_cast<std::size_t>(mCanvasDims.x) * mCanvasDims.y; i++) {
        if (col.Difference(curr_lay.GetPixel(i)) <= threshold) {
            selected_pixels[i] = true;
        }
    }
}

void LayerControl::DoCurrentTool(PreviewLayer& preview_layer, Tool& tool,
                                 PreviewLayer& preview_layer_for_selection,
                                 int color_selection_threshold /*= 0*/) {
    switch (tool.GetToolType()) {
    case ToolType::kRectShape: {
        auto points =
            HandleRectShape(preview_layer, Color::FromImVec4(tool.GetColor()));

        if (points.has_value()) {
            GetCurrentLayer().DrawRect(points->first, points->second,
                                       Layer::DrawType::kFill);
            preview_layer.Clear();
            MarkHistoryForUpdate();
        }
        return;
    }

    case ToolType::kSelectionTool: {
        auto points = HandleSelectionTool(preview_layer);

        if (points.has_value()) {
            mSelection.AddToSelection(points->first, points->second);
            preview_layer.Clear();
            preview_layer_for_selection.DrawRect(points->first, points->second,
                                                 kColorSelectionPreview);
        }
        return;
    }

    case ToolType::kColorSelection:
        HandleColorSelectionTool(preview_layer_for_selection,
                                 color_selection_threshold);
        return;

    case ToolType::kMoveSelection: {
        HandleMoveSelectionTool(preview_layer, preview_layer_for_selection);
        return;
    }

    default:
        break;
    }

    if (GetCurrentLayer().DoCurrentTool()) {
        MarkHistoryForUpdate();
    }
}

void LayerControl::AddLayer(Tool& tool, Camera& camera) {
    mCurrentCapture->layers.emplace_back(tool, camera, mSelection, mPboBuff,
                                         mCanvasDims);
    MarkHistoryForUpdate();
}

void LayerControl::MoveUp(std::size_t layer_index) {
    if (layer_index == 0) {
        return;
    }

    auto it1 = GetLayers().begin();
    std::advance(it1, layer_index);
    auto it2 = GetLayers().begin();
    std::advance(it2, layer_index - 1);
    std::iter_swap(it1, it2);

    if (mCurrentLayerIndex == layer_index) {
        mCurrentLayerIndex--;
    } else if (mCurrentLayerIndex == layer_index - 1) {
        mCurrentLayerIndex++;
    }
}

void LayerControl::MoveDown(std::size_t layer_index) {
    if (layer_index >= GetLayers().size() - 1) {
        return;
    }

    auto it1 = GetLayers().begin();
    std::advance(it1, layer_index);
    auto it2 = GetLayers().begin();
    std::advance(it2, layer_index + 1);
    std::iter_swap(it1, it2);

    if (mCurrentLayerIndex == layer_index) {
        mCurrentLayerIndex++;
    } else if (mCurrentLayerIndex == layer_index + 1) {
        mCurrentLayerIndex--;
    }
}

auto LayerControl::AtIndex(std::size_t index) -> Layer& {
    assert(index < GetLayers().size());

    auto iter = GetLayers().begin();
    std::advance(iter, index);

    return *iter;
}

void LayerControl::ResetDataToDefault() {
    GetLayers().clear();
    mCurrentLayerIndex = 0;
}

auto LayerControl::GetDisplayedCanvas() const -> std::vector<Color> {
    auto canvas_width = GetCanvasDims().x;
    auto canvas_height = GetCanvasDims().y;

    std::vector<Color> displayed_canvas{
        static_cast<std::size_t>(canvas_height * canvas_width)};
    std::vector<Color> layer_texture_data;

    for (const auto& layer_traversed : std::ranges::reverse_view(GetLayers())) {
        layer_traversed.GetTextureData(layer_texture_data);
        auto canvas_dims = layer_traversed.GetCanvasDims();

        for (int i = 0; i < canvas_height; i++) {
            for (int j = 0; j < canvas_width; j++) {
                Color pixel = layer_texture_data[(i * canvas_dims.x) + j];

                Color dst_color = {
                    .r = pixel.r,
                    .g = pixel.g,
                    .b = pixel.b,
                    .a = pixel.a == 0
                             ? pixel.a
                             : static_cast<uint8_t>(layer_traversed.mOpacity)};

                displayed_canvas[(i * canvas_width) + j] = Color::BlendColor(
                    dst_color, displayed_canvas[(i * canvas_width) + j]);
            }
        }
    }

    return displayed_canvas;
}

void LayerControl::PushToHistory() {
    assert(mCurrentCapture.has_value());
    assert(mCurrentUndoTreeNode != nullptr);

    mCurrentUndoTreeNode = &mCurrentUndoTreeNode->AddChild(
        mCurrentCapture->layers, mCurrentLayerIndex);
}

void LayerControl::Undo() {
    assert(mCurrentCapture.has_value());
    assert(mCurrentUndoTreeNode != nullptr);

    if (mCurrentUndoTreeNode->GetParent() == nullptr) {
        return;
    }

    mCurrentUndoTreeNode = mCurrentUndoTreeNode->GetParent();
    mCurrentCapture.emplace(mCurrentUndoTreeNode->GetData());
    mCurrentLayerIndex = mCurrentCapture->selected_layer_index;

    WriteCurrentLayerTextureDataToPbo();
}

void LayerControl::Redo() {
    assert(mCurrentCapture.has_value());
    assert(mCurrentUndoTreeNode != nullptr);

    auto& children = mCurrentUndoTreeNode->GetChildren();
    std::size_t child_last_used_index =
        mCurrentUndoTreeNode->GetLastUsedNodeIndex();

    if (children.size() == 0) {
        return;
    }

    if (child_last_used_index < children.size() - 1) {
        std::puts("Incorrect behaviour in Layers::Redo for now. Using the "
                  "first child");
        mCurrentUndoTreeNode = children.front().get();
        mCurrentCapture.emplace(mCurrentUndoTreeNode->GetData());
        return;
    }

    mCurrentUndoTreeNode = children[child_last_used_index].get();
    mCurrentCapture.emplace(mCurrentUndoTreeNode->GetData());
    mCurrentLayerIndex = mCurrentCapture->selected_layer_index;

    WriteCurrentLayerTextureDataToPbo();
}

// NOTE: This doesn't set last used child id
void LayerControl::SetCurrentNode(Tree<Capture>& node_to_set_to) {
    mCurrentUndoTreeNode = &node_to_set_to;
    mCurrentCapture.emplace(mCurrentUndoTreeNode->GetData());
    mCurrentLayerIndex = mCurrentCapture->selected_layer_index;
}

void LayerControl::UpdateAndDraw(bool should_do_tool, Tool& tool,
                                 Camera& camera, PreviewLayer& preview_layer,
                                 PreviewLayer& preview_layer_for_selection,
                                 Gla::PixelBuffer& pbo,
                                 int color_selection_threshold) {
    for (auto& layer : GetLayers()) {
        layer.Update();
    }

    if (should_do_tool) {
        DoCurrentTool(preview_layer, tool, preview_layer_for_selection,
                      color_selection_threshold);
    }

    if (GetCurrentLayer().IsEdited()) {
        auto& lay_tex = GetCurrentLayerTexture();
        pbo.Bind();
        lay_tex.Bind();

        pbo.Unmap();
        lay_tex.UpdateWholeTexture(mCanvasDims, nullptr);
        UpdateMappedPBOMemorySpanForAllLayers(pbo.Map());

        lay_tex.Unbind();
        Gla::PixelBuffer::Unbind();
    }

    if ((Events::IsCtrlPressed() &&
         Events::IsKeyboardKeyPressedDelayed(GLFW_KEY_Z)) ||
        mShouldUndo) {
        Undo();
    }

    if ((Events::IsCtrlPressed() &&
         Events::IsKeyboardKeyPressedDelayed(GLFW_KEY_R)) ||
        mShouldRedo) {
        Redo();
    }

    if (mShouldAddLayer) {
        AddLayer(tool, camera);
    }

    if (mShouldUpdateHistory) {
        PushToHistory();
    }

    mShouldUpdateHistory = false;
    mShouldUndo = false;
    mShouldRedo = false;
    mShouldAddLayer = false;
    mCurrentLayerIndexTemp = mCurrentLayerIndex;
    mPreviousToolType = GetCurrentToolType();
}

void LayerControl::InitHistory(Camera& camera, Tool& tool) {
    mCurrentCapture.emplace(tool, camera, mSelection, mPboBuff, mCanvasDims, 0);
    mUndoTree.emplace(auto{mCurrentCapture.value()});
    mCurrentUndoTreeNode = &(*mUndoTree);
    mSelection.Reset(mCanvasDims);
}

void LayerControl::WriteCurrentLayerTextureDataToPbo() {
    GetCurrentLayer().GetTexture().Bind();
    glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_UNSIGNED_BYTE, mPboBuff.data());
    GetCurrentLayer().GetTexture().Unbind();
}

void LayerControl::UpdateMappedPBOMemorySpanForAllLayers(
    Gla::PboMappedBuffSpan pbo_buff) {
    mPboBuff = pbo_buff;

    for (auto& layer : GetLayers()) {
        layer.UpdateMappedPBOBufferSpan(pbo_buff);
    }
}

void LayerControl::MoveSelectedPixelsInCurrentLayer(Vec2 offset) {
    std::vector<bool>& selected_pixels = mSelection.GetSelectedPixels();
    bool old_val = mSelection.ShouldCheckForSelection();
    mSelection.SetShouldCheckForSelectionValue(false);

    // A fourth of the canvas size seems like a pretty optimal size; the vector
    // won't need to resize in most cases, and in the worst case it will resize
    // twice
    Vec2 dims_one_fourth = mCanvasDims / 2;
    std::vector<std::pair<Vec2, Color>> pixels_to_overwrite;
    pixels_to_overwrite.reserve(static_cast<std::size_t>(dims_one_fourth.x) *
                                dims_one_fourth.y);

    auto& curr_lay = GetCurrentLayer();

    for (int i = 0; i < mCanvasDims.y; i++) {
        for (int j = 0; j < mCanvasDims.x; j++) {
            if (!selected_pixels[(i * mCanvasDims.x) + j]) {
                continue;
            }

            selected_pixels[(i * mCanvasDims.x) + j] = false;

            Vec2 coords{j, i};
            Vec2 coords_to_move_to = coords + offset;

            if (!AreCoordsInBounds(coords_to_move_to)) {
                continue;
            }

            Color pixel_to_move_color = curr_lay.GetPixel(coords);
            pixels_to_overwrite.emplace_back(coords_to_move_to,
                                             pixel_to_move_color);
            curr_lay.DrawPixel(coords, kColorTransparent);
        }
    }

    for (auto [coords, color] : pixels_to_overwrite) {
        curr_lay.DrawPixel(coords, color);
        selected_pixels[(coords.y * mCanvasDims.x) + coords.x] = true;
    }

    mSelection.SetShouldCheckForSelectionValue(old_val);
}

void LayerControl::UpdatePreviewLayerForSelection(
    PreviewLayer& preview_layer_for_selection) const {
    preview_layer_for_selection.Clear();
    const std::vector<bool>& selected_pixels = mSelection.GetSelectedPixels();

    for (std::size_t i = 0;
         i < static_cast<std::size_t>(mCanvasDims.x) * mCanvasDims.y; i++) {
        if (selected_pixels[i]) {
            preview_layer_for_selection.DrawPixel(i, kColorSelectionPreview);
        }
    }
}

auto LayerControl::AreCoordsInBounds(Vec2 coords) const -> bool {
    return (coords.x >= 0 && coords.y >= 0 && coords.x < mCanvasDims.x &&
            coords.y < mCanvasDims.y);
}
} // namespace Pikzel
