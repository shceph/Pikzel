#include "Events.hpp"

#include "GLFW/glfw3.h"

#include <cassert>
#include <chrono>

namespace Pikzel
{
void Events::GlfwScrollCallback(GLFWwindow* /*window*/, double xoffset,
                                double yoffset)
{
    for (auto& callable : sScrollCallbacks)
    {
        callable(xoffset, yoffset);
    }
}

void Events::GlfwCursorPosCallback(GLFWwindow* /*window*/, double x_pos,
                                   double y_pos)
{
    for (auto& callable : sCursorPosCallbacks)
    {
        callable(x_pos, y_pos);
    }
}

void Events::PushToScrollCallback(CallbackType&& callback)
{
    sScrollCallbacks.emplace_back(std::move(callback));
}

void Events::PushToCursorPosCallback(CallbackType&& callback)
{
    sCursorPosCallbacks.emplace_back(std::move(callback));
}

auto Events::IsKeyboardKeyPressed(KeyboardKey key) -> bool
{
    constexpr auto kDelay = std::chrono::milliseconds(130);
    auto& last_time_keyboard_used = GetLastTimeKeyboardUsed();

    if (std::chrono::steady_clock::now() - last_time_keyboard_used <= kDelay)
    {
        return false;
    }

    if (glfwGetKey(sWindow, key) == GLFW_PRESS)
    {
        last_time_keyboard_used = std::chrono::steady_clock::now();
        return true;
    }

    return false;
}

auto Events::IsMouseButtonPressed(MouseButtons button) -> bool
{
    assert(button != kMouseButtonCount);
    return glfwGetMouseButton(sWindow, button) == GLFW_PRESS;
}

auto Events::IsMouseButtonHeld(MouseButtons button) -> bool
{
    constexpr auto kDelay = std::chrono::milliseconds(25);
    assert(button != kMouseButtonCount);
    auto& last_time_clicked = GetLastTimeClickedArrayForEachButton();

    return glfwGetMouseButton(sWindow, button) == GLFW_PRESS &&
           std::chrono::steady_clock::now() - last_time_clicked.at(button) <=
               kDelay;
}

void Events::Update()
{
    auto& last_time_clicked = GetLastTimeClickedArrayForEachButton();

    for (auto i = 0UZ; i <= last_time_clicked.size(); i++)
    {
        if (glfwGetMouseButton(sWindow, static_cast<int>(i)) == GLFW_PRESS)
        {
            last_time_clicked.at(i) = std::chrono::steady_clock::now();
        }
    }

    glfwPollEvents();
}
} // namespace Pikzel
