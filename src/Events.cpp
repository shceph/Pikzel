#include "Events.hpp"

#include "Camera.hpp"
#include "GLFW/glfw3.h"

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
    return glfwGetKey(sWindow, key) == GLFW_PRESS;
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

    return glfwGetMouseButton(sWindow, button) == GLFW_PRESS &&
           std::chrono::steady_clock::now() - sLastTimeClicked.at(button) <=
               kDelay;
}

void Events::Update()
{
    for (auto i = 0UZ; i <= sLastTimeClicked.size(); i++)
    {
        if (glfwGetMouseButton(sWindow, static_cast<int>(i)) == GLFW_PRESS)
        {
            sLastTimeClicked.at(i) = std::chrono::steady_clock::now();
        }
    }

    glfwPollEvents();
}
} // namespace Pikzel
