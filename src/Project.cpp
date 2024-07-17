#include "Project.hpp"
#include "Application.hpp"
#include "Layer.hpp"
#include "Tool.hpp"

#include <stb/stb_image.h>
#include <stb/stb_image_write.h>
#include <stb/stb_image_resize2.h>

#include <fstream>
#include <iostream>
#include <string>

namespace App
{
static auto SaveImageToPNG(const char* filename, int width, int height,
                           int num_channels, const unsigned char* data) -> bool
{
    return stbi_write_png(filename, width, height, num_channels, data,
                          width * num_channels) != 0;
}

void Project::New(Vec2Int canvas_dims)
{
    if (sProjectOpened)
    {
        // TODO(scheph): Should handle the case if the user is trying to create
        // a new project whilst having one already opened. Maybe I could make a
        // prompt waringng the user and telling them the projrct won't be saved
        // if they continue;
    }

    sCanvasWidth = canvas_dims.x;
    sCanvasHeight = canvas_dims.y;

    sProjectOpened = true;

    Tool::SetDataToDefault();
    Layers::AddLayer();
}

void Project::Open(const std::string& /*project_file_dest*/)
{
}

void Project::SaveAsImage(int magnify_factor, const std::string& save_dest)
{
    constexpr int kChannelCount = 4;
    int arr_height = sCanvasHeight * magnify_factor;
    int arr_width = sCanvasWidth * magnify_factor * kChannelCount;
    std::vector<unsigned char> image_data(
        static_cast<size_t>(arr_height * arr_width));

    const CanvasData canvas_displayed = Layers::GetDisplayedCanvas();

    for (int i = 0; i < sCanvasHeight; i++)
    {
        for (int j = 0; j < sCanvasWidth; j++)
        {
            if (canvas_displayed[i][j].a > 1.0F)
            {
                for (int k = 0; k < magnify_factor; k++)
                {
                    for (int l = 0; l < magnify_factor; l++)
                    {
                        image_data[(i * magnify_factor + k) * arr_width +
                                   j * magnify_factor * kChannelCount +
                                   l * kChannelCount + 0] =
                            static_cast<unsigned char>(0);
                        image_data[(i * magnify_factor + k) * arr_width +
                                   j * magnify_factor * kChannelCount +
                                   l * kChannelCount + 1] =
                            static_cast<unsigned char>(0);
                        image_data[(i * magnify_factor + k) * arr_width +
                                   j * magnify_factor * kChannelCount +
                                   l * kChannelCount + 2] =
                            static_cast<unsigned char>(0);
                        image_data[(i * magnify_factor + k) * arr_width +
                                   j * magnify_factor * kChannelCount +
                                   l * kChannelCount + 3] =
                            static_cast<unsigned char>(0);
                    }
                }

                continue;
            }

            for (int k = 0; k < magnify_factor; k++)
            {
                for (int l = 0; l < magnify_factor; l++)
                {
                    image_data[(i * magnify_factor + k) * arr_width +
                               j * magnify_factor * kChannelCount +
                               l * kChannelCount + 0] =
                        static_cast<unsigned char>(canvas_displayed[i][j].r *
                                                   255);
                    image_data[(i * magnify_factor + k) * arr_width +
                               j * magnify_factor * kChannelCount +
                               l * kChannelCount + 1] =
                        static_cast<unsigned char>(canvas_displayed[i][j].g *
                                                   255);
                    image_data[(i * magnify_factor + k) * arr_width +
                               j * magnify_factor * kChannelCount +
                               l * kChannelCount + 2] =
                        static_cast<unsigned char>(canvas_displayed[i][j].b *
                                                   255);
                    image_data[(i * magnify_factor + k) * arr_width +
                               j * magnify_factor * kChannelCount +
                               l * kChannelCount + 3] =
                        static_cast<unsigned char>(canvas_displayed[i][j].a *
                                                   255);
                }
            }
        }
    }

    int height = sCanvasHeight * magnify_factor;
    int width = sCanvasWidth * magnify_factor;

    if (!SaveImageToPNG(save_dest.c_str(), width, height, kChannelCount,
                        image_data.data()))
    {
        UI::sRenderSaveErrorPopup = true;
    }
}

void Project::SaveAsProject(const std::string& save_dest)
{
    std::ofstream save_file(save_dest);

    if (!save_file.is_open())
    {
        // TODO(shceph): this is only a temproary way to handle the error
        std::cerr << "Couldn't open the file in "
                     "Project::SaveAsProject(std::string save_dest)"
                  << "\nFile: " << __FILE__ << "\nLine: " << __LINE__
                  << std::endl;

        return;
    }

    save_file.close();
}

void Project::CloseCurrentProject()
{
    Layers::ResetDataToDefault();
    Tool::SetDataToDefault();
    sProjectOpened = false;
}
} // namespace App
