#include "Application.hpp"
#include "Project.hpp"

#include <stb/stb_image.h>
#include <stb/stb_image_write.h>

#include <iostream>
#include <string>
#include <fstream>

namespace App
{
    static bool SaveImageToPNG(const char* filename, int width, int height, int num_channels, const unsigned char* data)
    {
        return stbi_write_png(filename, width, height, num_channels, data, width * num_channels);
    }

    void Project::New(int canvas_height, int canvas_width)
    {
        if (s_ProjectOpened)
        {
            // TODO: Should handle the case if the user is trying to create a new project whilst having one already opened.
            // Maybe I could make a prompt waringng the user and telling them the projrct won't be saved if they continue;
        }

        s_CanvasHeight = canvas_height;
        s_CanvasWidth  = canvas_width;

        s_ProjectOpened = true;

        Layers::AddLayer();
    }

    void Project::Open(std::string project_file_dest)
    {

    }

    void Project::SaveAsImage(int magnify_factor, std::string save_dest)
	{
        constexpr int CHANNEL_COUNT = 4;
        int arr_height = s_CanvasHeight * magnify_factor;
        int arr_width = s_CanvasWidth * magnify_factor * CHANNEL_COUNT;
        std::vector<unsigned char> image_data(arr_height * arr_width);

        const CanvasData& canvas_displayed = Layers::GetDisplayedCanvas();

        for (int i = 0; i < s_CanvasHeight; i++)
        {
            for (int j = 0; j < s_CanvasWidth; j++)
            {
                if (canvas_displayed[i][j].a > 1.0f)
                {
                    for (int k = 0; k < magnify_factor; k++)
                    {
                        for (int l = 0; l < magnify_factor; l++)
                        {
                            image_data[(i * magnify_factor + k) * arr_width + j * magnify_factor * CHANNEL_COUNT + l * CHANNEL_COUNT + 0] = (unsigned char)0;
                            image_data[(i * magnify_factor + k) * arr_width + j * magnify_factor * CHANNEL_COUNT + l * CHANNEL_COUNT + 1] = (unsigned char)0;
                            image_data[(i * magnify_factor + k) * arr_width + j * magnify_factor * CHANNEL_COUNT + l * CHANNEL_COUNT + 2] = (unsigned char)0;
                            image_data[(i * magnify_factor + k) * arr_width + j * magnify_factor * CHANNEL_COUNT + l * CHANNEL_COUNT + 3] = (unsigned char)0;
                        }
                    }

                    continue;
                }

                for (int k = 0; k < magnify_factor; k++)
                {
                    for (int l = 0; l < magnify_factor; l++)
                    {
                        image_data[(i * magnify_factor + k) * arr_width + j * magnify_factor * CHANNEL_COUNT + l * CHANNEL_COUNT + 0] = (unsigned char)(canvas_displayed[i][j].r * 255);
                        image_data[(i * magnify_factor + k) * arr_width + j * magnify_factor * CHANNEL_COUNT + l * CHANNEL_COUNT + 1] = (unsigned char)(canvas_displayed[i][j].g * 255);
                        image_data[(i * magnify_factor + k) * arr_width + j * magnify_factor * CHANNEL_COUNT + l * CHANNEL_COUNT + 2] = (unsigned char)(canvas_displayed[i][j].b * 255);
                        image_data[(i * magnify_factor + k) * arr_width + j * magnify_factor * CHANNEL_COUNT + l * CHANNEL_COUNT + 3] = (unsigned char)(canvas_displayed[i][j].a * 255);
                    }
                }
            }
        }

        int height = s_CanvasHeight * magnify_factor;
        int width = s_CanvasWidth * magnify_factor;

        if (!SaveImageToPNG(save_dest.c_str(), width, height, CHANNEL_COUNT, image_data.data()))
        {
            UI::s_RenderSaveErrorPopup = true;
        }
	}

    void Project::SaveAsProject(std::string save_dest)
    {
        std::ofstream save_file(save_dest);

        if (!save_file.is_open())
        {
            // TODO: this is only a temproary way to handle the error
            std::cerr << "Couldn't open the file in Project::SaveAsProject(std::string save_dest)"
                << "\nFile: " << __FILE__
                << "\nLine: " << __LINE__ 
                << std::endl;

            return;
        }

        save_file.close();
    }

    void Project::CloseCurrentProject()
    {
        Layers::ResetDataToDefault();
        Tool  ::ResetDataToDefault();
        s_ProjectOpened = false;
    }
}
