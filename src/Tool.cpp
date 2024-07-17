#include "Tool.hpp"

namespace App
{
    void Tool::SetDataToDefault()
    {
        GetColor1() = { 0.0F, 0.0F, 0.0F, 1.0F };
        GetColor2() = { 0.0F, 0.0F, 0.0F, 1.0F };
        sCurrentColor = &GetColor1();

        sCurrentToolType = kBrush;

        sBrushRadius = 1;
        sSelectedColorSlot = kColorSlot1;
    }
}  // namespace App
