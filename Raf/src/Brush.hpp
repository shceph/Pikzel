#pragma once

#include <imgui.h>

namespace App
{
	class Brush
	{
	public:
		constexpr static ImVec4  GetColor()			{ return *current_color; }
		constexpr static ImVec4& GetColorRef()		{ return *current_color; }

		constexpr static ImVec4& GetColor1()		{ return color_1; }
		constexpr static ImVec4& GetColor2()		{ return color_2; }

		constexpr static void SetCurrentColorToColor1() { current_color = &color_1; }
		constexpr static void SetCurrentColorToColor2() { current_color = &color_2; }

		enum BrushType { NORMAL_DRAW, DELETE };
		constexpr static BrushType GetCurrentBrushType() { return current_brush_type; }

		constexpr static int GetBrushRadius()			{ return brush_radius; }
		static int SetBrushRadius(int radius)			{ brush_radius = radius; }

		friend class UI;
	private:
		inline static ImVec4 color_1 = { 0.0f, 0.0f, 0.0f, 1.0f };
		inline static ImVec4 color_2 = { 0.0f, 0.0f, 0.0f, 1.0f };
		inline static ImVec4* current_color = &color_1;

		inline static BrushType current_brush_type = NORMAL_DRAW;

		inline static int brush_radius = 1;
	};
}