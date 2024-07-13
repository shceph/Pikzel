#pragma once
#include <chrono>

using namespace std::chrono_literals;

namespace Gla
{
    class Timer
    {
    public:
        Timer()
            : start(std::chrono::high_resolution_clock::now()) {}

        ~Timer() = default;

        // Returns time in seconds
        inline float GetTime() { return std::chrono::duration<float>(std::chrono::high_resolution_clock::now() - start).count(); }
        inline void  Reset()   { start = std::chrono::high_resolution_clock::now(); }

        static inline void  CalculateDeltaTime(float frame_time) { deltaTime = frame_time; }
        static inline float DeltaTime() { return deltaTime; }
        // Normalized so when the FPS is 60 'DeltaTimeNormalized' returns 1.0f
        static inline float DeltaTimeNormalized() { return deltaTime * 60.0f; }

        static constexpr float FPS60_frame_time = 1 / 60.0f;

    private:
        std::chrono::high_resolution_clock::time_point start;
        static float deltaTime;
    };

    inline float Timer::deltaTime = 0.0f;
}