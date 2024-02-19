#pragma once

#include <imgui.h>
#include <string>
#include <vector>
#include <list>

namespace App
{
    constexpr int CANVAS_HEIGHT = 32;
    constexpr int CANVAS_WIDTH = 32;

    struct Color
    {
        float r = 0.0f, g = 0.0f, b = 0.0f, a = 1.1f;

        void operator=(const ImVec4& color);
        bool operator==(const Color& other);
    };

    using CanvasData = std::vector<std::vector<Color>>;

    class Layer
    {
    public:
        Layer(int width, int height);
        void DoCurrentTool();
        void EmplaceVertices(std::vector<float>& vertices);

        constexpr bool IsVisible() const { return m_Visible; }
        constexpr bool IsLocked()  const { return m_Locked;  }

        constexpr void SwitchVisibilityState()  { m_Visible = !m_Visible; }
        constexpr void SwitchLockState()        { m_Locked = !m_Locked; }

        constexpr const std::string& GetName() const { return m_LayerName; }

    private:
        void DrawCircle(int center_x, int center_y, int radius, bool only_outline);

        int m_Width, m_Height;
        CanvasData m_Canvas;

        bool m_Visible = true, m_Locked = false;
        int m_Opacity = 255;

        std::string m_LayerName;

        friend class UI;
        friend class Layers;
    };

    class Layers
    {
    public:
        static Layer& GetCurrentLayer();

        static void DoCurrentTool();
        static void MoveUp(int layer_index);
        static void MoveDown(int layer_index);
        static void AddLayer();
        static void EmplaceVertices(std::vector<float>& vertices);

        static Layer& AtIndex(int index);

        static CanvasData& GetCanvas();
        static int GetLayerCount() { return layers.size(); }

    private:
        inline static std::list<Layer> layers = { Layer(CANVAS_HEIGHT, CANVAS_WIDTH) };
        inline static int current_layer_index = 0;

        friend class UI;
        friend class Layer;
    };
}