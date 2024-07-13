#pragma once

#include <string>

namespace App
{
	class Project
	{
	public:
		static void New(int canvas_heigth, int canvas_width);
		static void Open(std::string project_file_dest);
		static void SaveAsImage(int magnify_factor, std::string save_dest);
		static void SaveAsProject(std::string save_dest);
		static void CloseCurrentProject();

		inline static bool IsOpened()		{ return s_ProjectOpened; }
		inline static int  CanvasHeight()	{ return s_CanvasHeight;  }
		inline static int  CanvasWidth()	{ return s_CanvasWidth;   }

	private:
		inline static bool s_ProjectOpened = false;
		inline static int  s_CanvasHeight  = 0;
		inline static int  s_CanvasWidth   = 0;
	};
}
