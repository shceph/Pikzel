#include "tool.hpp"

namespace Pikzel
{
Tool::Tool() : mColor1{0.0F, 0.0F, 0.0F, 1.0F}, mColor2{0.0F, 0.0F, 0.0F, 1.0F}
{
}

void Tool::SetDataToDefault()
{
    mColor1 = {0.0F, 0.0F, 0.0F, 1.0F};
    mColor2 = {0.0F, 0.0F, 0.0F, 1.0F};
    mCurrentToolType = ToolType::kBrush;
    mBrushRadius = 1;
    mSelectedColorSlot = kColorSlot1;
}

auto Tool::GetColorRef() -> ImVec4&
{
    switch (mSelectedColorSlot)
    {
    case kColorSlot1:
        return mColor1;
    case kColorSlot2:
        return mColor2;
    default:
        assert(false);
        return mColor1;
    }
}

auto Tool::GetColorRef() const -> const ImVec4&
{
    switch (mSelectedColorSlot)
    {
    case kColorSlot1:
        return mColor1;
    case kColorSlot2:
        return mColor2;
    default:
        assert(false);
        return mColor1;
    }
}

auto Tool::GetColor() const -> ImVec4
{
    switch (mSelectedColorSlot)
    {
    case kColorSlot1:
        return mColor1;
    case kColorSlot2:
        return mColor2;
    default:
        assert(false);
        return mColor1;
    }
}
} // namespace Pikzel
