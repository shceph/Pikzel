#include "Gla/VertexBuffer.hpp"

#include "Layer.hpp"
#include "Project.hpp"
#include "VertexBufferControl.hpp"

namespace Pikzel
{
VertexBufferControl::VertexBufferControl(Vertex* ptr_to_buffer,
                                         std::size_t count)
{
    assert(Project::IsOpened());
    mBufferData = std::span<Vertex>(ptr_to_buffer, count);
    mVertexCount = count;
    sUpdateAll = true;
}

void VertexBufferControl::Update(bool should_update_all,
                                 const std::vector<Vec2Int>& dirty_pixels)
{
    assert(Project::IsOpened());

    if (should_update_all)
    {
        const auto& capture = Layers::GetCapture();

        for (const auto& layer : capture.layers)
        {
            std::size_t layer_index = 0;
            std::size_t offset = 0;

            for (int i = 0; i < Project::CanvasHeight(); i++)
            {
                for (int j = 0; j < Project::CanvasWidth(); j++)
                {
                    const auto color = layer.GetPixel({j, i});
                    const auto x_flt = static_cast<float>(j);
                    const auto y_flt = static_cast<float>(i);
                    const auto index =
                        offset + static_cast<std::size_t>(
                                     i * Project::CanvasWidth() + j) *
                                     kVerticesPerPixel;
                    // first triangle
                    // upper left corner
                    mBufferData[index] = Vertex{x_flt, y_flt, color};
                    // upper right corner
                    mBufferData[index + 1] = Vertex{x_flt + 1, y_flt, color};
                    // bottom left corner
                    mBufferData[index + 2] = Vertex{x_flt, y_flt + 1, color};
                    // second triangle
                    // upper right corner
                    mBufferData[index + 3] = Vertex{x_flt + 1, y_flt, color};
                    // bottom right corner
                    mBufferData[index + 4] =
                        Vertex{x_flt + 1, y_flt + 1, color};
                    // bottom left corner
                    mBufferData[index + 5] = Vertex{x_flt, y_flt + 1, color};
                }
            }

            layer_index++;
            offset += layer_index * Project::CanvasWidth() *
                      Project::CanvasHeight() * kVerticesPerPixel;
        }

        return;
    }

    const auto& layer = Layers::GetCurrentLayer();

    if (dirty_pixels.empty()) { return; }

    const auto offset = Layers::GetCurrentLayerIndex() *
                        Project::CanvasWidth() * Project::CanvasHeight() *
                        kVerticesPerPixel;

    for (const auto px_coords : dirty_pixels)
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
        mBufferData[index] = Vertex{x_flt, y_flt, color};
        // upper right corner
        mBufferData[index + 1] = Vertex{x_flt + 1, y_flt, color};
        // bottom left corner
        mBufferData[index + 2] = Vertex{x_flt, y_flt + 1, color};
        // second triangle
        // upper right corner
        mBufferData[index + 3] = Vertex{x_flt + 1, y_flt, color};
        // bottom right corner
        mBufferData[index + 4] = Vertex{x_flt + 1, y_flt + 1, color};
        // bottom left corner
        mBufferData[index + 5] = Vertex{x_flt, y_flt + 1, color};
    }
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
        kOffset +
        static_cast<std::size_t>(coords.y * Project::CanvasWidth() + coords.x) *
            kVerticesPerPixel;

    if (index > max_index) { return; }

    const auto color = Layers::GetCurrentLayer().GetPixel(coords);
    const auto x_flt = static_cast<float>(coords.x);
    const auto y_flt = static_cast<float>(coords.y);

    // first triangle
    // upper left corner
    mBufferData[index] = Vertex{x_flt, y_flt, color};
    // upper right corner
    mBufferData[index + 1] = Vertex{x_flt + 1, y_flt, color};
    // bottom left corner
    mBufferData[index + 2] = Vertex{x_flt, y_flt + 1, color};
    // second triangle
    // upper right corner
    mBufferData[index + 3] = Vertex{x_flt + 1, y_flt, color};
    // bottom right corner
    mBufferData[index + 4] = Vertex{x_flt + 1, y_flt + 1, color};
    // bottom left corner
    mBufferData[index + 5] = Vertex{x_flt, y_flt + 1, color};
}

void VertexBufferControl::UpdateSize(Gla::VertexBuffer& vbo)
{
    vbo.Bind();
    glUnmapBuffer(GL_ARRAY_BUFFER);

    auto vbo_size = GetNeededVBOSizeForLayer() * Layers::GetLayerCount();
    vbo.UpdateSizeIfNeeded(vbo_size);

    auto vertex_count = vbo_size / sizeof(Vertex);

    std::vector<Vertex> vertices(vertex_count);
    Layers::EmplaceVertices(vertices);
    assert(vertices.size() == vertex_count);
    vbo.UpdateData(vertices.data(), vbo_size);

    auto* ptr_to_buffer =
        static_cast<Vertex*>(glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY));

    mBufferData = std::span<Vertex>(ptr_to_buffer, vertex_count);
    mVertexCount = vertex_count;
}

void VertexBufferControl::UpdateSizeIfNeeded(Gla::VertexBuffer& vbo)
{
    if (vbo.GetSize() != GetNeededVBOSizeForLayer() * Layers::GetLayerCount())
    {
        UpdateSize(vbo);
    }
}

} // namespace Pikzel
