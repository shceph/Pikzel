#include "Tool.hpp"

namespace Pikzel
{
Tool::Tool()
    : mColor1{0.0F, 0.0F, 0.0F, 1.0F}, mColor2{0.0F, 0.0F, 0.0F, 1.0F},
      mCurrentColor{&mColor1}
{
}
} // namespace Pikzel
