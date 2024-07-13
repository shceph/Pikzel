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
        float r = 0.0f, g = 0.0f, b = 0.0f;
        float a = 1.1f;  // Alpha value greater than 1.0f means there's no color

        void operator=(const ImVec4& color);
        bool operator==(const Color& other)  const;
        bool operator==(const ImVec4& color) const;

        static Color BlendColor(Color col1, Color col2);
        static Color FromImVec4(const ImVec4 color);
    };

    using CanvasData = std::vector<std::vector<Color>>;

    class Layer
    {
    public:
        Layer();
        void DoCurrentTool();
        // Places the vertices containing the canvas data for my shader program
        void EmplaceVertices(std::vector<float>& vertices);

        inline bool IsVisible() const { return m_Visible; }
        inline bool IsLocked()  const { return m_Locked;  }

        inline void SwitchVisibilityState()  { m_Visible = !m_Visible; }
        inline void SwitchLockState()        { m_Locked = !m_Locked; }

        inline const std::string& GetName() const { return m_LayerName; }

    private:
        void DrawCircle(int center_x, int center_y, int radius, bool only_outline);
        void Fill(int x, int y, Color clicked_color);

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
        static void ResetDataToDefault();

        static Layer& AtIndex(int index);

        static const CanvasData GetDisplayedCanvas();
        inline static std::size_t GetLayerCount() { return s_Layers.size(); }

    private:
        inline static std::list<Layer> s_Layers;
        inline static int s_CurrentLayerIndex = 0;

        friend class UI;
        friend class Layer;
    };
}