#pragma once

#include <GLFW/glfw3.h>

#include <array>
#include <chrono>
#include <functional>
#include <vector>

namespace Pikzel
{
class Events
{
  public:
    using CallbackType = std::function<void(double, double)>;
    // Just int for now; use GLFW macros
    using KeyboardKey = int;
    enum MouseButtons
    {
        kButtonLeft = GLFW_MOUSE_BUTTON_LEFT,
        kButtonRight = GLFW_MOUSE_BUTTON_RIGHT,
        kButtonMiddle = GLFW_MOUSE_BUTTON_MIDDLE,
        kMouseButtonCount
    };

    static void GlfwScrollCallback(GLFWwindow* window, double xoffset,
                                   double yoffset);
    static void GlfwCursorPosCallback(GLFWwindow* window, double x_pos,
                                      double y_pos);
    static void PushToScrollCallback(CallbackType&& callback);
    static void PushToCursorPosCallback(CallbackType&& callback);
    static auto IsKeyboardKeyPressed(KeyboardKey key) -> bool;
    static auto IsMouseButtonPressed(MouseButtons button) -> bool;
    static auto IsMouseButtonHeld(MouseButtons button) -> bool;
    static void Update();

    static void SetWindowPtr(GLFWwindow* window) { sWindow = window; }

  private:
    inline static GLFWwindow* sWindow = nullptr;
    inline static std::vector<CallbackType> sScrollCallbacks;
    inline static std::vector<CallbackType> sCursorPosCallbacks;
    inline static std::array<std::chrono::time_point<std::chrono::steady_clock>,
                             kMouseButtonCount>
        sLastTimeClicked;
};
} // namespace Pikzel
