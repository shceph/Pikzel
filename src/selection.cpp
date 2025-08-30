#include "selection.hpp"

#include <cstddef>
#include <vector>
#include <algorithm>

#include <glm/ext/vector_int2.hpp>

namespace Pikzel {
void Selection::AddToSelection(glm::ivec2 upper_left, glm::ivec2 bottom_right) {
    auto min_x = std::min(upper_left.x, bottom_right.x);
    auto max_x = std::max(upper_left.x, bottom_right.x);
    auto min_y = std::min(upper_left.y, bottom_right.y);
    auto max_y = std::max(upper_left.y, bottom_right.y);

    for (int i = min_y; i <= max_y; i++) {
        for (int j = min_x; j <= max_x; j++) {
            mSelected[(i * mCanvasDims.x) + j] = true;
        }
    }

    mCheckForSelection = true;
}

void Selection::Clear() {
    std::ranges::fill(mSelected, false);
    mCheckForSelection = false;
}

auto Selection::IsPixelSelected(glm::ivec2 px_coords) -> bool {
    if (!mCheckForSelection) {
        return true;
    }

    return mSelected[(px_coords.y * mCanvasDims.x) + px_coords.x];
}

auto Selection::ShouldCheckForSelection() const -> bool {
    return mCheckForSelection;
}

void Selection::Reset(glm::ivec2 canvas_dims) {
    auto new_size = static_cast<std::size_t>(canvas_dims.x) * canvas_dims.y;
    mSelected.resize(new_size, false);
    mCanvasDims = canvas_dims;
    mCheckForSelection = false;
}

auto Selection::GetSelectedPixels() const -> const std::vector<bool>& {
    return mSelected;
}

auto Selection::GetSelectedPixels() -> std::vector<bool>& { return mSelected; }

void Selection::SetShouldCheckForSelectionValue(bool val) {
    mCheckForSelection = val;
}
} // namespace Pikzel
