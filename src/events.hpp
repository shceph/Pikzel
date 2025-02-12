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
    enum class MouseButtons : std::uint8_t
    {
        kButtonLeft = GLFW_MOUSE_BUTTON_LEFT,
        kButtonRight = GLFW_MOUSE_BUTTON_RIGHT,
        kButtonMiddle = GLFW_MOUSE_BUTTON_MIDDLE,
        kMouseButtonCount = 3
    };

    using CallbackType = std::function<void(double, double)>;
    using TimePointType = std::chrono::time_point<std::chrono::steady_clock>;
    using ArrayOfTimePoints =
        std::array<TimePointType,
                   static_cast<int>(MouseButtons::kMouseButtonCount)>;

    // Just int for now; use GLFW macros
    using KeyboardKey = int;
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

    static auto GetLastTimeClickedArrayForEachButton() -> ArrayOfTimePoints&
    {
        static ArrayOfTimePoints last_time_clicked;
        return last_time_clicked;
    }

    static auto GetLastTimeKeyboardUsed() -> TimePointType&
    {
        static TimePointType last_time_keyboard_used{
            std::chrono::steady_clock::now()};
        return last_time_keyboard_used;
    }

    template <typename... Args>
    static auto AreKeyboardKeysPressed(Args... args) -> bool
    {
        constexpr auto kDelay = std::chrono::milliseconds(130);
        auto& last_time_keyboard_used = GetLastTimeKeyboardUsed();

        if (std::chrono::steady_clock::now() - last_time_keyboard_used <=
            kDelay)
        {
            return false;
        }

        if (((glfwGetKey(sWindow, args) == GLFW_PRESS) && ...))
        {
            last_time_keyboard_used = std::chrono::steady_clock::now();
            return true;
        }

        return false;
    }

  private:
    inline static GLFWwindow* sWindow = nullptr;
    inline static std::vector<CallbackType> sScrollCallbacks;
    inline static std::vector<CallbackType> sCursorPosCallbacks;
};
} // namespace Pikzel
