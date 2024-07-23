#include "Project.hpp"
#include "Application.hpp"
#include "Layer.hpp"
#include "Tool.hpp"
#include "Camera.hpp"

#include <stb/stb_image.h>
#include <stb/stb_image_resize2.h>
#include <stb/stb_image_write.h>

#include <fstream>
#include <iostream>
#include <string>

namespace Pikzel
{
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
    Layers::InitHistory();
    Camera::SetCenter({sCanvasWidth / 2, sCanvasHeight / 2});
}

void Project::Open(const std::string& /*project_file_dest*/)
{
}

void Project::SaveAsImage(int magnify_factor, const std::string& save_dest)
{
    constexpr int kChannelCount = 4;
    int arr_height = sCanvasHeight * magnify_factor;
    int arr_width = sCanvasWidth * magnify_factor * kChannelCount;
    std::vector<uint8_t> image_data(
        static_cast<size_t>(arr_height * arr_width));

    const CanvasData canvas_displayed = Layers::GetDisplayedCanvas();

    for (int i = 0; i < sCanvasHeight; i++)
    {
        for (int j = 0; j < sCanvasWidth; j++)
        {
            for (int k = 0; k < magnify_factor; k++)
            {
                // NOLINTNEXTLINE(readability-identifier-length)
                for (int l = 0; l < magnify_factor; l++)
                {
                    image_data[(i * magnify_factor + k) * arr_width +
                               j * magnify_factor * kChannelCount +
                               l * kChannelCount + 0] =
                        canvas_displayed[i][j].r;
                    image_data[(i * magnify_factor + k) * arr_width +
                               j * magnify_factor * kChannelCount +
                               l * kChannelCount + 1] =
                        canvas_displayed[i][j].g;
                    image_data[(i * magnify_factor + k) * arr_width +
                               j * magnify_factor * kChannelCount +
                               l * kChannelCount + 2] =
                        canvas_displayed[i][j].b;
                    image_data[(i * magnify_factor + k) * arr_width +
                               j * magnify_factor * kChannelCount +
                               l * kChannelCount + 3] =
                        canvas_displayed[i][j].a;
                }
            }
        }
    }

    /*stbir_resize_uint8_srgb(&canvas_displayed[0][0].r, sCanvasWidth,
                            sCanvasHeight, sCanvasWidth * kChannelCount,
                            image_data.data(), sCanvasWidth * magnify_factor,
                            sCanvasHeight * magnify_factor, arr_width,
                            stbir_pixel_layout::STBIR_RGBA);*/

    int height = sCanvasHeight * magnify_factor;
    int width = sCanvasWidth * magnify_factor;

    if (stbi_write_png(save_dest.c_str(), width, height, kChannelCount,
                       image_data.data(), width * kChannelCount) == 0)
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
} // namespace Pikzel
