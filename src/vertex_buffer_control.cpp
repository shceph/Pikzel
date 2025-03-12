#include "gla/vertex_buffer.hpp"

#include "layer.hpp"
#include "project.hpp"
#include "vertex_buffer_control.hpp"
#include <cstddef>

namespace Pikzel
{
VertexBufferControl::VertexBufferControl(Layers& layers, Vertex* ptr_to_buffer,
                                         std::size_t count)
    : mLayers{layers}, mBufferData{ptr_to_buffer, count}, mVertexCount{count}
{
    sUpdateAll = true;
}

void VertexBufferControl::Map(Gla::VertexBuffer& vbo)
{
    vbo.Bind();

    auto vbo_size = GetNeededVBOSizeForLayer(mLayers.get().GetCanvasDims()) *
                    mLayers.get().GetLayerCount();
    auto vertex_count = vbo_size / sizeof(Vertex);

    auto* ptr_to_buffer =
        static_cast<Vertex*>(glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY));

    mBufferData = std::span<Vertex>(ptr_to_buffer, vertex_count);
    mVertexCount = vertex_count;
}

void VertexBufferControl::Unmap(Gla::VertexBuffer& vbo)
{
    vbo.Bind();
    glUnmapBuffer(GL_ARRAY_BUFFER);
}

void VertexBufferControl::Update(bool should_update_all,
                                 const std::vector<Vec2Int>& dirty_pixels)
{
    auto canvas_dims = mLayers.get().GetCanvasDims();

    if (should_update_all)
    {
        const auto& capture = mLayers.get().GetCapture();

        for (const auto& layer : capture.layers)
        {
            std::size_t layer_index = 0;
            std::size_t offset = 0;

            for (int i = 0; i < canvas_dims.y; i++)
            {
                for (int j = 0; j < canvas_dims.x; j++)
                {
                    const auto color = layer.GetPixel({j, i});
                    const auto x_flt = static_cast<float>(j);
                    const auto y_flt = static_cast<float>(i);
                    const auto index =
                        offset +
                        (static_cast<std::size_t>((i * canvas_dims.x) + j) *
                         kVerticesPerPixel);
                    // first triangle
                    // upper left corner
                    mBufferData[index] =
                        Vertex{.pos_x = x_flt, .pos_y = y_flt, .color = color};
                    // upper right corner
                    mBufferData[index + 1] = Vertex{
                        .pos_x = x_flt + 1, .pos_y = y_flt, .color = color};
                    // bottom left corner
                    mBufferData[index + 2] = Vertex{
                        .pos_x = x_flt, .pos_y = y_flt + 1, .color = color};
                    // second triangle
                    // upper right corner
                    mBufferData[index + 3] = Vertex{
                        .pos_x = x_flt + 1, .pos_y = y_flt, .color = color};
                    // bottom right corner
                    mBufferData[index + 4] = Vertex{
                        .pos_x = x_flt + 1, .pos_y = y_flt + 1, .color = color};
                    // bottom left corner
                    mBufferData[index + 5] = Vertex{
                        .pos_x = x_flt, .pos_y = y_flt + 1, .color = color};
                }
            }

            layer_index++;
            offset +=
                layer_index * canvas_dims.x * canvas_dims.y * kVerticesPerPixel;
        }

        return;
    }

    if (dirty_pixels.empty()) { return; }

    const auto& layer = mLayers.get().GetCurrentLayer();
    const auto offset = mLayers.get().GetCurrentLayerIndex() * canvas_dims.x *
                        canvas_dims.y * kVerticesPerPixel;

    for (const auto px_coords : dirty_pixels)
    {
        const auto index =
            offset + (static_cast<std::size_t>((px_coords.y * canvas_dims.x) +
                                               px_coords.x) *
                      kVerticesPerPixel);
        const auto color = layer.GetPixel(px_coords);
        const auto x_flt = static_cast<float>(px_coords.x);
        const auto y_flt = static_cast<float>(px_coords.y);

        // first triangle
        // upper left corner
        mBufferData[index] =
            Vertex{.pos_x = x_flt, .pos_y = y_flt, .color = color};
        // upper right corner
        mBufferData[index + 1] =
            Vertex{.pos_x = x_flt + 1, .pos_y = y_flt, .color = color};
        // bottom left corner
        mBufferData[index + 2] =
            Vertex{.pos_x = x_flt, .pos_y = y_flt + 1, .color = color};
        // second triangle
        // upper right corner
        mBufferData[index + 3] =
            Vertex{.pos_x = x_flt + 1, .pos_y = y_flt, .color = color};
        // bottom right corner
        mBufferData[index + 4] =
            Vertex{.pos_x = x_flt + 1, .pos_y = y_flt + 1, .color = color};
        // bottom left corner
        mBufferData[index + 5] =
            Vertex{.pos_x = x_flt, .pos_y = y_flt + 1, .color = color};
    }
}

void VertexBufferControl::PushDirtyPixel(Vec2Int dirty_pixel)
{
    sDirtyPixels.push_back(dirty_pixel);
}

void VertexBufferControl::UpdateSize(Gla::VertexBuffer& vbo)
{
    vbo.Bind();
    glUnmapBuffer(GL_ARRAY_BUFFER);

    auto vbo_size = GetNeededVBOSizeForLayer(mLayers.get().GetCanvasDims()) *
                    mLayers.get().GetLayerCount();
    vbo.UpdateSizeIfNeeded(vbo_size);

    auto vertex_count = vbo_size / sizeof(Vertex);

    std::vector<Vertex> vertices(vertex_count);
    mLayers.get().EmplaceVertices(vertices);
    assert(vertices.size() == vertex_count);
    vbo.UpdateData(vertices.data(), vbo_size);

    auto* ptr_to_buffer =
        static_cast<Vertex*>(glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY));

    mBufferData = std::span<Vertex>(ptr_to_buffer, vertex_count);
    mVertexCount = vertex_count;
}

void VertexBufferControl::UpdateSizeIfNeeded(Gla::VertexBuffer& vbo)
{
    if (vbo.GetSize() !=
        GetNeededVBOSizeForLayer(mLayers.get().GetCanvasDims()) *
            mLayers.get().GetLayerCount())
    {
        UpdateSize(vbo);
    }
}

} // namespace Pikzel
