#include "Tool.hpp"

namespace App
{
    void Tool::ResetDataToDefault()
    {
        s_Color1 = { 0.0f, 0.0f, 0.0f, 1.0f };
        s_Color2 = { 0.0f, 0.0f, 0.0f, 1.0f };
        s_CurrentColor = &s_Color1;

        s_CurrentToolType = BRUSH;

        s_BrushRadius = 1;
        s_SelectedColorSlot = COLOR_SLOT_1;
    }
}