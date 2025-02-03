#pragma once

#include <imgui.h>

namespace Pikzel
{
enum ToolType
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

    inline auto GetColor() -> ImVec4 { return *mCurrentColor; }
    inline auto GetColorRef() -> ImVec4& { return *mCurrentColor; }

    inline auto GetColor1() -> ImVec4& { return mColor2; }
    inline auto GetColor2() -> ImVec4& { return mColor1; }

    inline void SetCurrentColorToColor1()
    {
        mSelectedColorSlot = kColorSlot1;
        mCurrentColor = &GetColor1();
    }
    inline void SetCurrentColorToColor2()
    {
        mSelectedColorSlot = kColorSlot2;
        mCurrentColor = &GetColor2();
    }

    [[nodiscard]] inline auto GetSelectedColorSlot() const -> int
    {
        return mSelectedColorSlot;
    }

    [[nodiscard]] inline auto GetToolType() const -> ToolType
    {
        return mCurrentToolType;
    }
    inline void SetToolType(ToolType type) { mCurrentToolType = type; }

    [[nodiscard]] inline auto GetBrushRadius() const -> int
    {
        return mBrushRadius;
    }
    inline void SetBrushRadius(int radius) { mBrushRadius = radius; }

    constexpr static int kColorSlot1 = 1, kColorSlot2 = 2;

    friend class UI;
    friend class Layer;
    friend class Layers;

  private:
    ImVec4 mColor1;
    ImVec4 mColor2;
    ImVec4* mCurrentColor = nullptr;

    ToolType mCurrentToolType = kBrush;
    int mBrushRadius = 1;
    int mSelectedColorSlot = kColorSlot1;
};
} // namespace Pikzel
