#pragma once

#include <imgui.h>

namespace App
{
	enum ToolType { BRUSH, ERASER, COLOR_PICKER, BUCKET };

	class Tool
	{
	public:
		inline static ImVec4  GetColor()		{ return *current_color; }
		inline static ImVec4& GetColorRef()		{ return *current_color; }

		inline static ImVec4& GetColor1()		{ return color_1; }
		inline static ImVec4& GetColor2()		{ return color_2; }

		inline static void SetCurrentColorToColor1() { selected_color_slot = COLOR_SLOT_1; current_color = &color_1; }
		inline static void SetCurrentColorToColor2() { selected_color_slot = COLOR_SLOT_2; current_color = &color_2; }

		inline static int GetSelectedColorSlot() { return selected_color_slot; }

		inline static ToolType GetToolType()			{ return current_tool_type; }
		inline static void SetToolType(ToolType type)	{ current_tool_type = type; }

		inline static int GetBrushRadius()				{ return brush_radius; }
		inline static void SetBrushRadius(int radius)	{ brush_radius = radius; }

		constexpr static int COLOR_SLOT_1 = 1, COLOR_SLOT_2 = 2;

		friend class UI;
		friend class Layer;
		friend class Layers;
	private:
		inline static ImVec4 color_1 = { 0.0f, 0.0f, 0.0f, 1.0f };
		inline static ImVec4 color_2 = { 0.0f, 0.0f, 0.0f, 1.0f };
		inline static ImVec4* current_color = &color_1;

		inline static ToolType current_tool_type = BRUSH;

		inline static int brush_radius = 1;
		inline static int selected_color_slot = COLOR_SLOT_1;
	};
}