#include "VertexBufferControl.hpp"
#include "Layer.hpp"
#include "Project.hpp"

namespace Pikzel
{
void VertexBufferControl::Init(Vertex* ptr_to_buffer, std::size_t count)
{
    assert(Project::IsOpened());
    sBufferData = std::span<Vertex>(ptr_to_buffer, count);
    sVertexCount = count;
    sUpdateAll = true;
}

void VertexBufferControl::Update()
{
    assert(Project::IsOpened());

    if (sUpdateAll)
    {
        const auto& capture = Layers::GetCapture();

        for (const auto& layer : capture.layers)
        {
            for (int i = 0; i < Project::CanvasHeight(); i++)
            {
                for (int j = 0; j < Project::CanvasWidth(); j++)
                {
                    const auto color = layer.GetPixel({j, i});
                    const auto x_flt = static_cast<float>(j);
                    const auto y_flt = static_cast<float>(i);
                    const auto index = static_cast<std::size_t>(
                                           i * Project::CanvasWidth() + j) *
                                       kVerticesPerPixel;
                    // first triangle
                    // upper left corner
                    sBufferData[index] = Vertex{x_flt, y_flt, color};
                    // upper right corner
                    sBufferData[index + 1] = Vertex{x_flt + 1, y_flt, color};
                    // bottom left corner
                    sBufferData[index + 2] = Vertex{x_flt, y_flt + 1, color};
                    // second triangle
                    // upper right corner
                    sBufferData[index + 3] = Vertex{x_flt + 1, y_flt, color};
                    // bottom right corner
                    sBufferData[index + 4] =
                        Vertex{x_flt + 1, y_flt + 1, color};
                    // bottom left corner
                    sBufferData[index + 5] = Vertex{x_flt, y_flt + 1, color};
                }
            }
        }

        sUpdateAll = false;
        return;
    }

    const auto& layer = Layers::GetCurrentLayer();
    /* const auto& dirty_pixels = Layers::GetCurrentLayer().GetDirtyPixels(); */

    if (sDirtyPixels.empty()) { return; }

    const auto offset = Layers::GetCurrentLayerIndex() *
                        Project::CanvasWidth() * Project::CanvasHeight() *
                        kVerticesPerPixel;

    for (const auto px_coords : sDirtyPixels)
    {
        const auto index =
            offset + static_cast<std::size_t>(
                         px_coords.y * Project::CanvasWidth() + px_coords.x) *
                         kVerticesPerPixel;
        const auto color = layer.GetPixel(px_coords);
        const auto x_flt = static_cast<float>(px_coords.x);
        const auto y_flt = static_cast<float>(px_coords.y);

        // first triangle
        // upper left corner
        sBufferData[index] = Vertex{x_flt, y_flt, color};
        // upper right corner
        sBufferData[index + 1] = Vertex{x_flt + 1, y_flt, color};
        // bottom left corner
        sBufferData[index + 2] = Vertex{x_flt, y_flt + 1, color};
        // second triangle
        // upper right corner
        sBufferData[index + 3] = Vertex{x_flt + 1, y_flt, color};
        // bottom right corner
        sBufferData[index + 4] = Vertex{x_flt + 1, y_flt + 1, color};
        // bottom left corner
        sBufferData[index + 5] = Vertex{x_flt, y_flt + 1, color};
    }

	sDirtyPixels.clear();
}

void VertexBufferControl::PushDirtyPixel(Vec2Int dirty_pixel)
{
	sDirtyPixels.push_back(dirty_pixel);
}

void VertexBufferControl::UpdatePixel(Vec2Int coords)
{
    const std::size_t max_index =
        Project::CanvasWidth() * Project::CanvasHeight() * kVerticesPerPixel -
        1;
    static const auto kOffset = Layers::GetCurrentLayerIndex() *
                                Project::CanvasWidth() *
                                Project::CanvasHeight() * kVerticesPerPixel;

    const auto index =
        kOffset + static_cast<std::size_t>(coords.y * Project::CanvasWidth() +
                                           coords.x) *
                      kVerticesPerPixel;

    if (index > max_index) { return; }

    const auto color = Layers::GetCurrentLayer().GetPixel(coords);
    const auto x_flt = static_cast<float>(coords.x);
    const auto y_flt = static_cast<float>(coords.y);

    // first triangle
    // upper left corner
    sBufferData[index] = Vertex{x_flt, y_flt, color};
    // upper right corner
    sBufferData[index + 1] = Vertex{x_flt + 1, y_flt, color};
    // bottom left corner
    sBufferData[index + 2] = Vertex{x_flt, y_flt + 1, color};
    // second triangle
    // upper right corner
    sBufferData[index + 3] = Vertex{x_flt + 1, y_flt, color};
    // bottom right corner
    sBufferData[index + 4] = Vertex{x_flt + 1, y_flt + 1, color};
    // bottom left corner
    sBufferData[index + 5] = Vertex{x_flt, y_flt + 1, color};
}

} // namespace Pikzel
