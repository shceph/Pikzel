#pragma once
#include <chrono>

namespace Gla
{
using namespace std::chrono_literals;

class Timer
{
  public:
    Timer() : mStart(std::chrono::high_resolution_clock::now()) {}

    Timer(const Timer&) = default;
    Timer(Timer&&) = delete;
    auto operator=(const Timer&) -> Timer& = default;
    auto operator=(Timer&&) -> Timer& = delete;
    ~Timer() = default;

    // Returns time in seconds
    inline auto GetTime() -> float
    {
        return std::chrono::duration<float>(
                   std::chrono::high_resolution_clock::now() - mStart)
            .count();
    }
    inline void Reset() { mStart = std::chrono::high_resolution_clock::now(); }

    static inline void CalculateDeltaTime(float frame_time)
    {
        sDeltaTime = frame_time;
    }
    static inline auto DeltaTime() -> float { return sDeltaTime; }
    // Normalized so when the FPS is 60 'DeltaTimeNormalized' returns 1.0f
    static inline auto DeltaTimeNormalized() -> float
    {
        return sDeltaTime * 60.0F;
    }

    static constexpr float kFpS60FrameTime = 1 / 60.0F;

  private:
    std::chrono::high_resolution_clock::time_point mStart;
    static float sDeltaTime;
};

inline float Timer::sDeltaTime = 0.0F;
} // namespace Gla
