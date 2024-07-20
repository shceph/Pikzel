#pragma once

#include <imgui.h>
#include <GLFW/glfw3.h>

#include "Definitions.hpp"
#include "Tool.hpp"
#include "Layer.hpp"
#include "Project.hpp"

namespace App
{
	class UI
	{
	public:
		static void ImGuiInit(GLFWwindow* _window);
		static void ImGuiCleanup();
		static void NewFrame();
		static void RenderAndEndFrame();
		static void RenderUI();
		static void RenderNoProjectWindow();
		static void RenderDrawWindow(unsigned int framebuffer_texture_id, const char* window_name);

		static void SetupToolTextures(unsigned int brush_tex_id, unsigned int eraser_tex_id, unsigned int color_pick_tex_id, unsigned int bucket_tex_id);
		static void SetupLayerToolTextures(unsigned int eye_opened_id, unsigned int eye_closed_id, unsigned int lock_locked_id, unsigned int lock_unlocked_id);

		static bool ShouldUpdateVertexBuffer();
		static bool ShouldDoTool();

		inline static const ImVec2& GetDrawWinUpperleftCoords()		{ return s_DrawWindowUpperleftCornerCoords; }
		inline static const ImVec2& GetDrawWinDimensions()			{ return s_DrawWindowDimensions; }

		inline static const ImVec2& GetCanvasUpperleftCoords()		{ return s_CanvasUpperleftCoords; }
		inline static const ImVec2& GetCanvasBottomRightCoords()	{ return s_CanvasBottomrightCoords; }

		inline static GLFWwindow* GetWindowPointer()	{ return s_Window; }

		inline static void SetVertexBuffUpdateToTrue()	{ s_UpdateVertexBuffer = true;  }
		inline static void SetVertexBuffUpdateToFalse()	{ s_UpdateVertexBuffer = false; }
		inline static void SetShouldDoToolToTrue()		{ s_ShouldDoTool       = true;	}

	private:
		static void RenderMainMenuBar();
		static void RenderSaveAsImagePopup();
		static void RenderSaveAsProjectPopup();
		static void RenderColorWindow();
		static void RenderColorPalette(ImVec4& color);
		static void RenderToolWindow();
		static void RenderLayerWindow();
		static void RenderSaveErrorPopup();
		static void RenderNewProjectPopup();

		static void BeginOutline(ImVec4& outline_color = s_SelectedItemOutlineColor);
		static void EndOutline();

		//static void Save(int, std::string);

		inline static GLFWwindow* s_Window;
		inline static ImVec2 s_DrawWindowUpperleftCornerCoords;
		inline static ImVec2 s_DrawWindowDimensions;
		inline static ImVec2 s_CanvasUpperleftCoords;
		inline static ImVec2 s_CanvasBottomrightCoords;

		inline static ImTextureID s_BrushToolTextureID			= nullptr;
		inline static ImTextureID s_EraserToolTextureID			= nullptr;
		inline static ImTextureID s_ColorPickerToolTextureID	= nullptr;
		inline static ImTextureID s_BucketToolTextureID			= nullptr;

		inline static ImTextureID s_EyeOpenedTextureID			= nullptr;
		inline static ImTextureID s_EyeClosedTextureID			= nullptr;
		inline static ImTextureID s_LockLockedTextureID			= nullptr;
		inline static ImTextureID s_LockUnlockedTextureID		= nullptr;

		inline static ImVec4 s_SelectedItemOutlineColor;

		// Says whether the vertex buffer should be updated, or the canvas state, in other words
		inline static bool s_UpdateVertexBuffer		= true;
		inline static bool s_ShouldDoTool			= false;
		inline static bool s_RenderSaveAsImgPopup	= false;
		inline static bool s_RenderSaveAsPrjPopup	= false;
		inline static bool s_RenderSaveErrorPopup	= false;
		inline static bool s_RenderNewProjectPopup	= false;

		friend void Project::SaveAsImage(int, std::string);
	};
}