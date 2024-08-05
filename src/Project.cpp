#include "Project.hpp"
#include "Application.hpp"
#include "Camera.hpp"
#include "Layer.hpp"
#include "Tool.hpp"

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
        // TODO: Should handle the case if the user is trying to create
        // a new project whilst having one already opened. Maybe I could make a
        // prompt waringng the user and telling them the projrct won't be saved
        // if they continue;
    }

    sCanvasWidth = canvas_dims.x;
    sCanvasHeight = canvas_dims.y;

    sProjectOpened = true;

    auto& old_tmp_layer = Layers::GetTempLayer();
    old_tmp_layer = Layer{};
    Layer::ResetConstructCounter();
    Tool::SetDataToDefault();
    Layers::InitHistory();
    Camera::SetCenter({sCanvasWidth / 2, sCanvasHeight / 2});
}

void Project::Open(const std::string& project_file_dest)
{
    std::ifstream proj_file(project_file_dest);

    if (!proj_file.is_open())
    {
#ifdef _DEBUG
        std::cerr << "Couldn't open the file: " << project_file_dest
                  << " in Project::Open(const std::string&)"
                  << "\nFile: " << __FILE__ << "\nLine: " << __LINE__ << '\n';
#endif
        return;
    }

    std::size_t layer_count = 0UZ;
    int width = 0;
    int height = 0;

    if (!(proj_file >> layer_count) || !(proj_file >> width) ||
        !(proj_file >> height))
    {
#ifdef _DEBUG
        std::cerr
            << "Invalid project file, at Project::Open(const std::string&)\n";

        if (proj_file.eof()) { std::cerr << "EOF reached\n"; }
#endif
        return;
    }

    Vec2Int canvas_dims{width, height};
    Project::New(canvas_dims);
    auto& layers = Layers::GetCapture().layers;
    layers.clear();
    Layer::ResetConstructCounter();

    for (auto lay = 0UZ; lay < layer_count; lay++)
    {
        layers.emplace_back();
        auto iter = layers.begin();
        std::advance(iter, lay);

        int opacity = 0;
        if (!(proj_file >> opacity))
        {
#ifdef _DEBUG
            std::cerr << "Invalid project file, at Project::Open(const "
                         "std::string&), copying opacity\n";
#endif
            return;
        }
        iter->mOpacity = opacity;

        /*std::string name;
        if (!std::getline(proj_file, name))
        {
#ifdef _DEBUG
            std::cerr << "Invalid project file, at Project::Open(const "
                         "std::string&), copying name\n";
#endif
            return;
        }
        iter->mLayerName = name;*/

        for (int i = 0; i < canvas_dims.y; i++)
        {
            for (int j = 0; j < canvas_dims.x; j++)
            {
                Color col;

                if (!(proj_file >> col.r) || !(proj_file >> col.g) ||
                    !(proj_file >> col.b) || !(proj_file >> col.a))
                {
#ifdef _DEBUG
                    std::cerr << "Invalid project file, at Project::Open(const "
                                 "std::string&), copying colors\n";
#endif
                    return;
                }

                iter->mCanvas[i][j] = col;
            }
        }
    }

    proj_file.close();
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
    std::ofstream save_file(save_dest + ".pkz");

    if (!save_file.is_open())
    {
#ifdef _DEBUG
        std::cerr << "Couldn't open the file in "
                     "Project::SaveAsProject(std::string save_dest)"
                  << "\nFile: " << __FILE__ << "\nLine: " << __LINE__ << '\n';
#endif
        return;
    }

    auto& layers = Layers::GetCapture().layers;
    save_file << static_cast<std::size_t>(layers.size()) << " ";
    save_file << Project::CanvasWidth() << " ";
    save_file << Project::CanvasHeight() << "\n";

    for (auto& layer : layers)
    {
        save_file << layer.GetOpacity() << "\n";
        // save_file << layer.GetName() << "\n";

        for (auto& canvas_row : layer.mCanvas)
        {
            for (Color elem : canvas_row)
            {
                save_file << elem.r << " ";
                save_file << elem.g << " ";
                save_file << elem.b << " ";
                save_file << elem.a << "\n";
            }
        }
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
