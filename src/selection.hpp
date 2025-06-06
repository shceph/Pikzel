#pragma once

#include <glm/glm.hpp>
#include <vector>

namespace Pikzel
{
class Selection
{
  public:
    void AddToSelection(glm::ivec2 upper_left, glm::ivec2 bottom_right);
    void Clear();
    auto IsPixelSelected(glm::ivec2 px_coords) -> bool;
    [[nodiscard]] auto ShouldCheckForSelection() const -> bool;
    void Reset(glm::ivec2 canvas_dims);
    [[nodiscard]] auto GetSelectedPixels() const -> const std::vector<bool>&;

  private:
    std::vector<bool> mSelected;
    glm::ivec2 mCanvasDims{0, 0};
    bool mCheckForSelection{false};
};
} // namespace Pikzel
