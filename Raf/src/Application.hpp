#pragma once

#include <imgui.h>
#include <GLFW/glfw3.h>

#include "Tool.hpp"
#include "Layer.hpp"

namespace App
{
	class UI
	{
	public:
		static void ImGuiInit(GLFWwindow* _window);
		static void Update();
		static void NewFrame();
		static void RenderAndEndFrame();
		static void RenderUI();
		static void RenderDrawWindow(unsigned int framebuffer_texture_id, const char* window_name);

		static void SetupToolTextures(unsigned int brush_tex_id, unsigned int eraser_tex_id, unsigned int color_pick_tex_id, unsigned int bucket_tex_id);
		static void SetupLayerToolTextures(unsigned int eye_opened_id, unsigned int eye_closed_id, unsigned int lock_locked_id, unsigned int lock_unlocked_id);

		static bool ShouldUpdateVertexBuffer();
		static bool DoTool();

		constexpr static const ImVec2& GetDrawWinUpperleftCoords()		{ return draw_window_upperleft_corner_coords; }
		constexpr static const ImVec2& GetDrawWinDimensions()			{ return draw_window_dimensions; }

		constexpr static const ImVec2& GetCanvasUpperleftCoords()		{ return canvas_upperleft_coords; }
		constexpr static const ImVec2& GetCanvasBottomRightCoords()		{ return canvas_bottomright_coords; }

		constexpr static GLFWwindow* GetWindowPointer()	{ return window; }

	private:
		static void RenderMainMenuBar();
		static void RenderColorWindow();
		static void RenderColorPalette(ImVec4& color);
		static void RenderToolWindow();
		static void RenderLayerWindow();

		static void BeginOutline(ImVec4& outline_color = selected_item_outline_color);
		static void EndOutline();

		inline static GLFWwindow* window;
		inline static ImVec2 draw_window_upperleft_corner_coords;
		inline static ImVec2 draw_window_dimensions;
		inline static ImVec2 canvas_upperleft_coords;
		inline static ImVec2 canvas_bottomright_coords;

		inline static ImTextureID brush_tool_texture_id			= nullptr;
		inline static ImTextureID eraser_tool_texture_id		= nullptr;
		inline static ImTextureID color_picker_tool_texture_id	= nullptr;
		inline static ImTextureID bucket_tool_texture_id		= nullptr;

		inline static ImTextureID eye_opened_texture_id			= nullptr;
		inline static ImTextureID eye_closed_texture_id			= nullptr;
		inline static ImTextureID lock_locked_texture_id		= nullptr;
		inline static ImTextureID lock_unlocked_texture_id		= nullptr;

		inline static ImVec4 selected_item_outline_color;

		// Says whether the vertex buffer should be updated, or the canvas state, in other words
		inline static bool update_vertex_buffer = false;
		inline static bool do_tool = false;
	};

	void Save();
}