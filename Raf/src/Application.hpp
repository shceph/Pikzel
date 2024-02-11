#pragma once

#include <imgui.h>
#include <GLFW/glfw3.h>

#include "Brush.hpp"

namespace App
{
	class UI
	{
	public:
		static void ImGuiInit(GLFWwindow* _window);
		static void NewFrame();
		static void RenderAndEndFrame();
		static void RenderUI();
		static void RenderDrawWindow(unsigned int framebuffer_texture_id, const char* window_name);

		constexpr static const ImVec2& GetDrawWinUpperleftCoords()		{ return draw_window_upperleft_corner_coords; }
		constexpr static const ImVec2& GetDrawWinDimensions()			{ return draw_window_dimensions; }

		constexpr static const ImVec2& GetCanvasUpperleftCoords()		{ return canvas_upperleft_coords; }
		constexpr static const ImVec2& GetCanvasBottomRightCoords()		{ return canvas_bottomright_coords; }

	private:
		static void RenderBrushWindow();
		static void RenderColorPalette(ImVec4& color);

		inline static GLFWwindow* window;
		inline static ImVec2 draw_window_upperleft_corner_coords;
		inline static ImVec2 draw_window_dimensions;
		inline static ImVec2 canvas_upperleft_coords;
		inline static ImVec2 canvas_bottomright_coords;
	};
}