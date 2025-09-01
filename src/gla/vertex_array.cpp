#include "vertex_array.hpp"

#include "vertex_buffer.hpp"
#include "vertex_buffer_layout.hpp"

#include <glad/gl.h>

#include <cstdint>
#include <bit>

namespace Gla {
VertexArray::VertexArray() { glGenVertexArrays(1, &mRendererID); }

VertexArray::~VertexArray() { glDeleteVertexArrays(1, &mRendererID); }

void VertexArray::AddBuffer(const VertexBuffer& vbo,
                            const VertexBufferLayout& layout) const {
    Bind();
    vbo.Bind();

    const auto& elements = layout.GetElements();
    unsigned int offset = 0;

    for (GLuint i = 0; i < elements.size(); i++) {
        const auto& element = elements[i];

        glEnableVertexAttribArray(i);
        glVertexAttribPointer(
            i, static_cast<GLsizei>(element.count), element.type,
            element.normalized, static_cast<GLsizei>(layout.GetStride()),
            std::bit_cast<const void*>(static_cast<uintptr_t>(offset)));

        offset +=
            element.count * VertexBufferElement::GetSizeOfType(element.type);
    }
}

void VertexArray::Bind() const { glBindVertexArray(mRendererID); }

void VertexArray::Unbind() { glBindVertexArray(0); }
} // namespace Gla
