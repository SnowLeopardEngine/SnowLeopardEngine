#include "SnowLeopardEngine/Core/Time/Timer.h"

namespace SnowLeopardEngine
{
    void Timer::Start() { m_LastFrameTime = std::chrono::high_resolution_clock::now(); }

    void Timer::Stop()
    {
        auto currentTime = std::chrono::high_resolution_clock::now();
        m_DeltaTime      = std::chrono::duration<float>(currentTime - m_LastFrameTime).count();
    }
} // namespace SnowLeopardEngine
