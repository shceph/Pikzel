#include "project.hpp"

#include <glad/gl.h>

#include <cstddef>
#include <cstdint>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>
#include <iterator>

#include <stb/stb_image.h>
#include <stb/stb_image_resize2.h>
#include <stb/stb_image_write.h>

#include "gla/pixel_buffer.hpp"

#include "camera.hpp"
#include "layer.hpp"
#include "layer_control.hpp"
#include "tool.hpp"

namespace Pikzel {
Project::Project(LayerControl& layers, Tool& tool, Camera& camera)
    : mLayers{layers}, mTool{tool}, mCamera{camera} {}

void Project::New(Vec2 canvas_dims) {
    if (mProjectOpened) {
        // TODO: Should handle the case if the user is trying to create
        // a new project whilst having one already opened. Maybe I could make a
        // prompt waringng the user and telling them the projrct won't be saved
        // if they continue;
    }

    mCanvasWidth = canvas_dims.x;
    mCanvasHeight = canvas_dims.y;

    mProjectOpened = true;

    Layer::ResetConstructCounter();
    mTool.get().SetDataToDefault();
    mLayers.get().SetCanvasDims(canvas_dims);
    mLayers.get().InitHistory(mCamera, mTool);
    mCamera.get().SetCanvasDims({mCanvasWidth, mCanvasHeight});
    mCamera.get().ResetCamera();
}

void Project::Open(const std::string& project_file_dest) {
    std::ifstream proj_file(project_file_dest);

    if (!proj_file.is_open()) {
#ifndef NDEBUG
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
        !(proj_file >> height)) {
#ifndef NDEBUG
        std::cerr
            << "Invalid project file, at Project::Open(const std::string&)\n";

        if (proj_file.eof()) {
            std::cerr << "EOF reached\n";
        }
#endif
        return;
    }

    const Vec2 canvas_dims{width, height};
    Project::New(canvas_dims);
    auto& layers = mLayers.get().GetCapture().layers;
    layers.clear();
    Layer::ResetConstructCounter();

    for (auto lay = 0UZ; lay < layer_count; lay++) {
        layers.emplace_back(mTool, mCamera, mLayers.get().mSelection,
                            mLayers.get().mPboBuff, canvas_dims);
        auto iter = layers.begin();
        std::advance(iter, lay);

        int opacity = 0;
        if (!(proj_file >> opacity)) {
#ifndef NDEBUG
            std::cerr << "Invalid project file, at Project::Open(const "
                         "std::string&), copying opacity\n";
#endif
            return;
        }
        iter->mOpacity = opacity;

        /*std::string name;
        if (!std::getline(proj_file, name))
        {
#ifndef NDEBUG
            std::cerr << "Invalid project file, at Project::Open(const "
                         "std::string&), copying name\n";
#endif
            return;
        }
        iter->mLayerName = name;*/

        for (int i = 0; i < canvas_dims.y; i++) {
            for (int j = 0; j < canvas_dims.x; j++) {
                Color col;

                if (!(proj_file >> col.r) || !(proj_file >> col.g) ||
                    !(proj_file >> col.b) || !(proj_file >> col.a)) {
#ifndef NDEBUG
                    std::cerr << "Invalid project file, at Project::Open(const "
                                 "std::string&), copying colors\n";
#endif
                    return;
                }

                iter->DrawPixel({j, i}, col);
            }
        }
    }

    proj_file.close();
}

auto Project::SaveAsImage(int magnify_factor,
                          const std::string& save_dest) const -> bool {
    constexpr int kChannelCount = 4;
    const int arr_height = mCanvasHeight * magnify_factor;
    const int arr_width = mCanvasWidth * magnify_factor * kChannelCount;
    std::vector<uint8_t> image_data(
        static_cast<size_t>(arr_height * arr_width));

    const std::vector<Color>& canvas_displayed =
        mLayers.get().GetDisplayedCanvas();

    for (int i = 0; i < mCanvasHeight; i++) {
        for (int j = 0; j < mCanvasWidth; j++) {
            const Color pixel_color = canvas_displayed[(i * mCanvasWidth) + j];
            for (int k = 0; k < magnify_factor; k++) {
                for (int l = 0; l < magnify_factor; l++) {
                    image_data[((i * magnify_factor + k) * arr_width) +
                               (j * magnify_factor * kChannelCount) +
                               (l * kChannelCount) + 0] = pixel_color.r;
                    image_data[((i * magnify_factor + k) * arr_width) +
                               (j * magnify_factor * kChannelCount) +
                               (l * kChannelCount) + 1] = pixel_color.g;
                    image_data[((i * magnify_factor + k) * arr_width) +
                               (j * magnify_factor * kChannelCount) +
                               (l * kChannelCount) + 2] = pixel_color.b;
                    image_data[((i * magnify_factor + k) * arr_width) +
                               (j * magnify_factor * kChannelCount) +
                               (l * kChannelCount) + 3] = pixel_color.a;
                }
            }
        }
    }

    const int height = mCanvasHeight * magnify_factor;
    const int width = mCanvasWidth * magnify_factor;

    return stbi_write_png(save_dest.c_str(), width, height, kChannelCount,
                          image_data.data(), width * kChannelCount) != 0;
}

void Project::SaveAsProject(const std::string& save_dest) {
    std::ofstream save_file(save_dest + ".pkz");

    if (!save_file.is_open()) {
#ifndef NDEBUG
        std::cerr << "Couldn't open the file in "
                     "Project::SaveAsProject(std::string save_dest)"
                  << "\nFile: " << __FILE__ << "\nLine: " << __LINE__ << '\n';
#endif
        return;
    }

    auto& layers = mLayers.get().GetCapture().layers;
    save_file << static_cast<std::size_t>(layers.size()) << " ";
    save_file << Project::CanvasWidth() << " ";
    save_file << Project::CanvasHeight() << "\n";

    std::vector<Gla::Color> texture_data{
        static_cast<std::size_t>(mCanvasHeight * mCanvasWidth)};

    for (auto& layer : layers) {
        save_file << layer.GetOpacity() << "\n";
        // save_file << layer.GetName() << "\n";
        layer.GetTexture().Bind();
        glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_UNSIGNED_BYTE,
                      texture_data.data());
        layer.GetTexture().Unbind();

        for (const Gla::Color col : texture_data) {
            save_file << col.r << " ";
            save_file << col.g << " ";
            save_file << col.b << " ";
            save_file << col.a << "\n";
        }
    }

    save_file.close();
}

void Project::CloseCurrentProject() {
    mLayers.get().ResetDataToDefault();
    mTool.get().SetDataToDefault();
    mProjectOpened = false;
}
} // namespace Pikzel
