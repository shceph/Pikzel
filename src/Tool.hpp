#pragma once

#include <imgui.h>

namespace App
{
	enum ToolType { BRUSH, ERASER, COLOR_PICKER, BUCKET };

	class Tool
	{
	public:
		inline static ImVec4  GetColor()		{ return *s_CurrentColor; }
		inline static ImVec4& GetColorRef()		{ return *s_CurrentColor; }

		inline static ImVec4& GetColor1()		{ return s_Color1; }
		inline static ImVec4& GetColor2()		{ return s_Color2; }

		inline static void SetCurrentColorToColor1() { s_SelectedColorSlot = COLOR_SLOT_1; s_CurrentColor = &s_Color1; }
		inline static void SetCurrentColorToColor2() { s_SelectedColorSlot = COLOR_SLOT_2; s_CurrentColor = &s_Color2; }

		inline static int GetSelectedColorSlot() { return s_SelectedColorSlot; }

		inline static ToolType GetToolType()			{ return s_CurrentToolType; }
		inline static void SetToolType(ToolType type)	{ s_CurrentToolType = type; }

		inline static int GetBrushRadius()				{ return s_BrushRadius; }
		inline static void SetBrushRadius(int radius)	{ s_BrushRadius = radius; }

		static void ResetDataToDefault();

		constexpr static int COLOR_SLOT_1 = 1, COLOR_SLOT_2 = 2;

		friend class UI;
		friend class Layer;
		friend class Layers;
	private:
		inline static ImVec4 s_Color1 = { 0.0f, 0.0f, 0.0f, 1.0f };
		inline static ImVec4 s_Color2 = { 0.0f, 0.0f, 0.0f, 1.0f };
		inline static ImVec4* s_CurrentColor = &s_Color1;

		inline static ToolType s_CurrentToolType = BRUSH;

		inline static int s_BrushRadius = 1;
		inline static int s_SelectedColorSlot = COLOR_SLOT_1;
	};
}