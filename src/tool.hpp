#pragma once

#include <cstdint>
#include <imgui.h>

namespace Pikzel
{
enum class ToolType : std::uint8_t
{
    kBrush,
    kEraser,
    kColorPicker,
    kBucket,
    kRectShape,
    kToolCount,
};

class Tool
{
  public:
    Tool();
    ~Tool() = default;
    Tool(const Tool&) = delete;
    Tool(Tool&&) = delete;
    auto operator=(Tool&&) -> Tool& = delete;
    auto operator=(const Tool&) -> Tool& = delete;

    void SetDataToDefault();
    [[nodiscard]] auto GetColorRef() -> ImVec4&;
    [[nodiscard]] auto GetColorRef() const -> const ImVec4&;
    [[nodiscard]] auto GetColor() const -> ImVec4;

    [[nodiscard]] auto GetColor1() const -> ImVec4 { return mColor1; }
    [[nodiscard]] auto GetColor2() const -> ImVec4 { return mColor2; }

    void SetColor1(ImVec4 color) { mColor1 = color; }
    void SetColor2(ImVec4 color) { mColor2 = color; }

    void SetCurrentColorToColor1() { mSelectedColorSlot = kColorSlot1; }
    void SetCurrentColorToColor2() { mSelectedColorSlot = kColorSlot2; }

    [[nodiscard]] auto GetSelectedColorSlot() const -> int
    {
        return mSelectedColorSlot;
    }

    [[nodiscard]] auto GetToolType() const -> ToolType
    {
        return mCurrentToolType;
    }
    void SetToolType(ToolType type) { mCurrentToolType = type; }

    [[nodiscard]] auto GetBrushRadius() const -> int { return mBrushRadius; }
    void SetBrushRadius(int radius) { mBrushRadius = radius; }

    constexpr static int kColorSlot1 = 1, kColorSlot2 = 2;

    friend class UI;
    friend class Layer;
    friend class Layers;

  private:
    ImVec4 mColor1;
    ImVec4 mColor2;

    ToolType mCurrentToolType = ToolType::kBrush;
    int mBrushRadius = 1;
    int mSelectedColorSlot = kColorSlot1;
};
} // namespace Pikzel
