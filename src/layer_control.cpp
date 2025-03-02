#include "layer_control.hpp"
#include "events.hpp"
#include "layer.hpp"
#include "vertex_buffer_control.hpp"

#include "GLFW/glfw3.h"
#include <cstddef>
#include <glm/geometric.hpp>

#include <algorithm>
#include <array>
#include <cmath>
#include <list>
#include <ranges>
#include <utility>
#include <vector>

namespace Pikzel
{

auto Layers::GetCurrentLayer() -> Layer&
{
    assert(mCurrentLayerIndex >= 0 && mCurrentLayerIndex < GetLayers().size());

    auto iter = GetLayers().begin();
    std::advance(iter, mCurrentLayerIndex);
    return *iter;
}

auto Layers::GetCanvasDims() const -> Vec2Int
{
    return mCanvasDims;
}

void Layers::DoCurrentTool()
{
    if (GetCurrentLayer().DoCurrentTool()) { MarkHistoryForUpdate(); }
}

void Layers::AddLayer(std::shared_ptr<Tool> tool,
                      std::shared_ptr<Camera> camera)
{
    GetLayers().emplace_back(std::move(tool), std::move(camera), mCanvasDims);
}

void Layers::MoveUp(std::size_t layer_index)
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

void Layers::MoveDown(std::size_t layer_index)
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

void Layers::EmplaceVertices(std::vector<Vertex>& vertices) const
{
    vertices.clear();

    for (const auto& layer : GetLayers())
    {
        layer.EmplaceVertices(vertices);
    }
}

void Layers::EmplaceBckgVertices(std::vector<Vertex>& vertices,
                                 std::optional<Vec2Int> custom_dims) const
{
    constexpr std::array<Color, 2> kBgColors = {
        Color{.r = 131, .g = 131, .b = 131, .a = 255},
        Color{.r = 201, .g = 201, .b = 201, .a = 255}};

    if (!custom_dims.has_value()) { custom_dims.emplace(GetCanvasDims()); }
    assert(custom_dims.has_value());

    auto canvas_width = custom_dims.value().x;
    auto canvas_height = custom_dims.value().y;

    for (int i = 0; i < canvas_height; i += 6)
    {
        for (int j = 0; j < canvas_width; j += 6)
        {
            auto x_coord = static_cast<float>(j);
            auto y_coord = static_cast<float>(i);
            glm::vec2 dims = custom_dims.value();

            // upper left corner
            vertices.emplace_back(x_coord, y_coord,
                                  kBgColors.at(((i + j) / 6) % 2));
            // upper right corner
            vertices.emplace_back(std::clamp(x_coord + 6, 0.0F, dims.x),
                                  y_coord, kBgColors.at(((i + j) / 6) % 2));
            // bottom left corner
            vertices.emplace_back(x_coord,
                                  std::clamp(y_coord + 6, 0.0F, dims.y),
                                  kBgColors.at(((i + j) / 6) % 2));
            /* second triangle */
            // upper right corner
            vertices.emplace_back(std::clamp(x_coord + 6, 0.0F, dims.x),
                                  y_coord, kBgColors.at(((i + j) / 6) % 2));
            // bottom right corner
            vertices.emplace_back(std::clamp(x_coord + 6, 0.0F, dims.x),
                                  std::clamp(y_coord + 6, 0.0F, dims.y),
                                  kBgColors.at(((i + j) / 6) % 2));
            // bottom left corner
            vertices.emplace_back(x_coord,
                                  std::clamp(y_coord + 6, 0.0F, dims.y),
                                  kBgColors.at(((i + j) / 6) % 2));
        }
    }
}

auto Layers::AtIndex(std::size_t index) -> Layer&
{
    assert(index >= 0 && index < GetLayers().size());

    auto iter = GetLayers().begin();
    std::advance(iter, index);

    return *iter;
}

void Layers::ResetDataToDefault()
{
    GetLayers().clear();
    mCurrentLayerIndex = 0;
}

auto Layers::GetDisplayedCanvas() const -> CanvasData
{
    auto canvas_width = GetCanvasDims().x;
    auto canvas_height = GetCanvasDims().y;

    CanvasData displayed_canvas(canvas_height,
                                std::vector<Color>(canvas_width));

    for (const auto& layer_traversed : std::ranges::reverse_view(GetLayers()))
    {
        for (int i = 0; i < canvas_width; i++)
        {
            for (int j = 0; j < canvas_height; j++)
            {
                Color pixel = layer_traversed.GetPixel({j, i});

                Color dst_color = {
                    .r = pixel.r,
                    .g = pixel.g,
                    .b = pixel.b,
                    .a = pixel.a == 0
                             ? pixel.a
                             : static_cast<uint8_t>(layer_traversed.mOpacity)};

                displayed_canvas[i][j] =
                    Color::BlendColor(dst_color, displayed_canvas[i][j]);
            }
        }
    }

    return displayed_canvas;
}

void Layers::PushToHistory()
{
    if (mCurrentCapture < mHistory.size() - 1)
    {
        auto delete_begin = mHistory.begin();
        std::advance(delete_begin, mCurrentCapture + 1);
        mHistory.erase(delete_begin, mHistory.end());
    }

    mHistory.emplace_back(GetLayers(), mCurrentLayerIndex);

    if (mHistory.size() > kMaxHistoryLenght)
    {
        auto delete_end =
            mHistory.begin() +
            static_cast<std::ptrdiff_t>(mHistory.size() - kMaxHistoryLenght);
        mHistory.erase(mHistory.begin(), delete_end);
    }

    mCurrentCapture = mHistory.size() - 1;
}

void Layers::Undo()
{
    if (mCurrentCapture != 0)
    {
        mCurrentCapture--;
        mCurrentLayerIndex = mHistory[mCurrentCapture].selected_layer_index;
        VertexBufferControl::SetUpdateAllToTrue();
        Layer::SetUpdateWholeVBOToTrue();
    }
}

void Layers::Redo()
{
    if (mCurrentCapture != mHistory.size() - 1)
    {
        mCurrentCapture++;
        mCurrentLayerIndex = mHistory[mCurrentCapture].selected_layer_index;
        VertexBufferControl::SetUpdateAllToTrue();
        Layer::SetUpdateWholeVBOToTrue();
    }
}

void Layers::UpdateAndDraw(bool should_do_tool, std::shared_ptr<Tool> tool,
                           std::shared_ptr<Camera> camera)
{
    for (auto& layer : GetLayers())
    {
        layer.Update();
    }

    if (should_do_tool) { DoCurrentTool(); }

    if (Events::AreKeyboardKeysPressed(GLFW_KEY_LEFT_CONTROL, GLFW_KEY_Z) ||
        mShouldUndo)
    {
        Undo();
    }

    if (Events::AreKeyboardKeysPressed(GLFW_KEY_LEFT_CONTROL, GLFW_KEY_R) ||
        mShouldRedo)
    {
        Redo();
    }

    if (mShouldAddLayer) { AddLayer(std::move(tool), std::move(camera)); }

    if (mShouldUpdateHistory) { PushToHistory(); }

    mShouldUpdateHistory = false;
    mShouldUndo = false;
    mShouldRedo = false;
    mShouldAddLayer = false;
}
} // namespace Pikzel
