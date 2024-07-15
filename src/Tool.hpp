#pragma once

#include <imgui.h>

namespace App
{
enum ToolType
{
    kBrush,
    kEraser,
    kColorPicker,
    kBucket
};

class Tool
{
  public:
    inline static auto GetColor() -> ImVec4
    {
        return *sCurrentColor;
    }
    inline static auto GetColorRef() -> ImVec4&
    {
        return *sCurrentColor;
    }

    inline static auto GetColor1() -> ImVec4&
    {
	   static ImVec4 color1{0.0F, 0.0F, 0.0F, 1.0F};
        return color1;
    }
    inline static auto GetColor2() -> ImVec4&
    {
	   static ImVec4 color2{0.0F, 0.0F, 0.0F, 1.0F};
        return color2;
    }

    inline static void SetCurrentColorToColor1()
    {
        sSelectedColorSlot = kColorSlot1;
        sCurrentColor = &GetColor1();
    }
    inline static void SetCurrentColorToColor2()
    {
        sSelectedColorSlot = kColorSlot2;
        sCurrentColor = &GetColor2();
    }

    inline static auto GetSelectedColorSlot() -> int
    {
        return sSelectedColorSlot;
    }

    inline static auto GetToolType() -> ToolType
    {
        return sCurrentToolType;
    }
    inline static void SetToolType(ToolType type)
    {
        sCurrentToolType = type;
    }

    inline static auto GetBrushRadius() -> int
    {
        return sBrushRadius;
    }
    inline static void SetBrushRadius(int radius)
    {
        sBrushRadius = radius;
    }

    static void ResetDataToDefault();

    constexpr static int kColorSlot1 = 1, kColorSlot2 = 2;

    friend class UI;
    friend class Layer;
    friend class Layers;

  private:
    inline static ImVec4* sCurrentColor = nullptr;

    inline static ToolType sCurrentToolType = kBrush;

    inline static int sBrushRadius = 1;
    inline static int sSelectedColorSlot = kColorSlot1;
};
} // namespace App
