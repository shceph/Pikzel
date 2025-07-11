#include "layer_control.hpp"
#include "events.hpp"
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

namespace Pikzel
{
auto LayerControl::GetCurrentLayer() -> Layer&
{
    assert(mCurrentLayerIndex >= 0 && mCurrentLayerIndex < GetLayers().size());

    auto iter = GetLayers().begin();
    std::advance(iter, mCurrentLayerIndex);
    return *iter;
}

auto LayerControl::GetCurrentLayer() const -> const Layer&
{
    assert(mCurrentLayerIndex >= 0 && mCurrentLayerIndex < GetLayers().size());

    auto iter = GetLayers().begin();
    std::advance(iter, mCurrentLayerIndex);
    return *iter;
}

auto LayerControl::GetCanvasDims() const -> Vec2Int
{
    return mCanvasDims;
}

auto LayerControl::HandleSelectionTool(PreviewLayer& preview_layer) const
    -> std::optional<std::pair<Vec2Int, Vec2Int>>
{
    auto canv_coord = CanvasCoordsFromCursorPos();
    if (!canv_coord.has_value()) { return std::nullopt; }
    bool left_button_pressed =
        Events::IsMouseButtonPressed(Events::MouseButtons::kButtonLeft);

    static bool shape_began = false;
    static Vec2Int shape_begin_coords{0, 0};

    if (!shape_began)
    {
        if (left_button_pressed)
        {
            shape_begin_coords = *canv_coord;
            shape_began = true;
        }
        return std::nullopt;
    }

    // Use left shift to force drawing a square
    if (Events::IsKeyboardKeyPressed(GLFW_KEY_LEFT_SHIFT))
    {
        int diff_x = shape_begin_coords.x - canv_coord->x;
        int diff_y = shape_begin_coords.y - canv_coord->y;

        if (std::abs(diff_x) < std::abs(diff_y))
        {
            canv_coord->y = shape_begin_coords.y - diff_x;
        }
        else { canv_coord->x = shape_begin_coords.x - diff_y; }
    }

    if (left_button_pressed)
    {
        preview_layer.Clear();
        preview_layer.DrawRect(shape_begin_coords, *canv_coord,
                               kColorSelectionPreview);

        return std::nullopt;
    }

    std::pair<Vec2Int, Vec2Int> ret{shape_begin_coords, *canv_coord};
    shape_began = false;
    shape_begin_coords = {0, 0};
    return ret;
}

auto LayerControl::HandleRectShape(PreviewLayer& preview_layer,
                                   Color tool_color) const
    -> std::optional<std::pair<Vec2Int, Vec2Int>>
{
    auto canv_coord = CanvasCoordsFromCursorPos();
    if (!canv_coord.has_value()) { return std::nullopt; }
    bool left_button_pressed =
        Events::IsMouseButtonPressed(Events::MouseButtons::kButtonLeft);

    static bool shape_began = false;
    static Vec2Int shape_begin_coords{0, 0};

    if (!shape_began)
    {
        if (left_button_pressed)
        {
            shape_begin_coords = *canv_coord;
            shape_began = true;
        }
        return std::nullopt;
    }

    // Use left shift to force drawing a square
    if (Events::IsKeyboardKeyPressed(GLFW_KEY_LEFT_SHIFT))
    {
        int diff_x = shape_begin_coords.x - canv_coord->x;
        int diff_y = shape_begin_coords.y - canv_coord->y;

        if (std::abs(diff_x) < std::abs(diff_y))
        {
            canv_coord->y = shape_begin_coords.y - diff_x;
        }
        else { canv_coord->x = shape_begin_coords.x - diff_y; }
    }

    if (left_button_pressed)
    {
        preview_layer.Clear();
        preview_layer.DrawRect(shape_begin_coords, *canv_coord, tool_color);

        return std::nullopt;
    }

    std::pair<Vec2Int, Vec2Int> ret{shape_begin_coords, *canv_coord};
    shape_began = false;
    shape_begin_coords = {0, 0};
    return ret;
}

void LayerControl::DoCurrentTool(PreviewLayer& preview_layer, Tool& tool,
                                 PreviewLayer& preview_layer_for_selection)
{
    if (tool.GetToolType() == ToolType::kSelectionTool)
    {
        auto points = HandleSelectionTool(preview_layer);

        if (points.has_value())
        {
            mSelection.AddToSelection(points->first, points->second);
            preview_layer.Clear();
            preview_layer_for_selection.DrawRect(points->first, points->second,
                                                 kColorSelectionPreview);
        }

        return;
    }

    if (tool.GetToolType() == ToolType::kRectShape)
    {
        auto points =
            HandleRectShape(preview_layer, Color::FromImVec4(tool.GetColor()));

        if (points.has_value())
        {
            GetCurrentLayer().DrawRect(points->first, points->second, true);
            preview_layer.Clear();
        }

        return;
    }

    if (GetCurrentLayer().DoCurrentTool()) { MarkHistoryForUpdate(); }
}

void LayerControl::AddLayer(Tool& tool, Camera& camera)
{
    mCurrentCapture->layers.emplace_back(tool, camera, mSelection, mPboBuff,
                                         mCanvasDims);
    MarkHistoryForUpdate();
}

void LayerControl::MoveUp(std::size_t layer_index)
{
    if (layer_index == 0) { return; }

    auto it1 = GetLayers().begin();
    std::advance(it1, layer_index);
    auto it2 = GetLayers().begin();
    std::advance(it2, layer_index - 1);
    std::iter_swap(it1, it2);

    if (mCurrentLayerIndex == layer_index) { mCurrentLayerIndex--; }
    else if (mCurrentLayerIndex == layer_index - 1) { mCurrentLayerIndex++; }
}

void LayerControl::MoveDown(std::size_t layer_index)
{
    if (layer_index >= GetLayers().size() - 1) { return; }

    auto it1 = GetLayers().begin();
    std::advance(it1, layer_index);
    auto it2 = GetLayers().begin();
    std::advance(it2, layer_index + 1);
    std::iter_swap(it1, it2);

    if (mCurrentLayerIndex == layer_index) { mCurrentLayerIndex++; }
    else if (mCurrentLayerIndex == layer_index + 1) { mCurrentLayerIndex--; }
}

auto LayerControl::AtIndex(std::size_t index) -> Layer&
{
    assert(index >= 0 && index < GetLayers().size());

    auto iter = GetLayers().begin();
    std::advance(iter, index);

    return *iter;
}

void LayerControl::ResetDataToDefault()
{
    GetLayers().clear();
    mCurrentLayerIndex = 0;
}

auto LayerControl::GetDisplayedCanvas() const -> std::vector<Color>
{
    auto canvas_width = GetCanvasDims().x;
    auto canvas_height = GetCanvasDims().y;

    std::vector<Color> displayed_canvas{
        static_cast<std::size_t>(canvas_height * canvas_width)};

    for (const auto& layer_traversed : std::ranges::reverse_view(GetLayers()))
    {
        for (int i = 0; i < canvas_height; i++)
        {
            for (int j = 0; j < canvas_width; j++)
            {
                Color pixel = layer_traversed.GetPixel({j, i});

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

void LayerControl::PushToHistory()
{
    assert(mCurrentCapture.has_value());
    assert(mCurrentUndoTreeNode != nullptr);

    mCurrentUndoTreeNode = &mCurrentUndoTreeNode->AddChild(
        mCurrentCapture->layers, mCurrentLayerIndex);
}

void LayerControl::Undo()
{
    assert(mCurrentCapture.has_value());
    assert(mCurrentUndoTreeNode != nullptr);

    if (mCurrentUndoTreeNode->GetParent() == nullptr) { return; }

    mCurrentUndoTreeNode = mCurrentUndoTreeNode->GetParent();
    mCurrentCapture.emplace(mCurrentUndoTreeNode->GetData());
    mCurrentLayerIndex = mCurrentCapture->selected_layer_index;
}

void LayerControl::Redo()
{
    assert(mCurrentCapture.has_value());
    assert(mCurrentUndoTreeNode != nullptr);

    auto& children = mCurrentUndoTreeNode->GetChildren();
    std::size_t child_last_used_index =
        mCurrentUndoTreeNode->GetLastUsedNodeIndex();

    if (children.size() == 0) { return; }

    if (child_last_used_index < children.size() - 1)
    {
        std::puts("Incorrect behaviour in Layers::Redo for now. Using the "
                  "first child");
        mCurrentUndoTreeNode = children.front().get();
        mCurrentCapture.emplace(mCurrentUndoTreeNode->GetData());
        return;
    }

    mCurrentUndoTreeNode = children[child_last_used_index].get();
    mCurrentCapture.emplace(mCurrentUndoTreeNode->GetData());
    mCurrentLayerIndex = mCurrentCapture->selected_layer_index;
}

// NOTE: This doesn't set last used child id
void LayerControl::SetCurrentNode(Tree<Capture>& node_to_set_to)
{
    mCurrentUndoTreeNode = &node_to_set_to;
    mCurrentCapture.emplace(mCurrentUndoTreeNode->GetData());
    mCurrentLayerIndex = mCurrentCapture->selected_layer_index;
}

void LayerControl::UpdateAndDraw(bool should_do_tool, Tool& tool,
                                 Camera& camera, PreviewLayer& preview_layer,
                                 PreviewLayer& preview_layer_for_selection)
{
    for (auto& layer : GetLayers())
    {
        layer.Update();
    }

    if (should_do_tool)
    {
        DoCurrentTool(preview_layer, tool, preview_layer_for_selection);
    }

    if ((Events::IsCtrlPressed() && Events::IsKeyboardKeyPressed(GLFW_KEY_Z)) ||
        mShouldUndo)
    {
        Undo();
    }

    if ((Events::IsCtrlPressed() && Events::IsKeyboardKeyPressed(GLFW_KEY_R)) ||
        mShouldRedo)
    {
        Redo();
    }

    if (mShouldAddLayer) { AddLayer(tool, camera); }

    if (mShouldUpdateHistory) { PushToHistory(); }

    mShouldUpdateHistory = false;
    mShouldUndo = false;
    mShouldRedo = false;
    mShouldAddLayer = false;
    mCurrentLayerIndexTemp = mCurrentLayerIndex;
}

void LayerControl::InitHistory(Camera& camera, Tool& tool)
{
    mCurrentCapture.emplace(tool, camera, mSelection, mPboBuff, mCanvasDims, 0);
    mUndoTree.emplace(auto{mCurrentCapture.value()});
    mCurrentUndoTreeNode = &(*mUndoTree);
    mSelection.Reset(mCanvasDims);
}

void LayerControl::WriteCurrentLayerTextureDataToPbo()
{
    GetCurrentLayer().GetTexture().Bind();
    glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_UNSIGNED_BYTE, mPboBuff.data());
    GetCurrentLayer().GetTexture().Unbind();
}

void LayerControl::UpdateMappedPBOMemorySpanForAllLayers(
    Gla::PboMappedBuffSpan pbo_buff)
{
    mPboBuff = pbo_buff;

    for (auto& layer : GetLayers())
    {
        layer.UpdateMappedPBOBufferSpan(pbo_buff);
    }
}
} // namespace Pikzel
