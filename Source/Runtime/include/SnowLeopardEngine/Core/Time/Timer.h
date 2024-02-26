#pragma once

#include "SnowLeopardEngine/Core/Base/Base.h"

namespace SnowLeopardEngine
{
    class Timer
    {
    public:
        /**
         * @brief Start timer
         *
         */
        void Start();

        /**
         * @brief Stop timer
         *
         */
        void Stop();

        /**
         * @brief Get the Delta Time
         *
         * @return float Delta time
         */
        float GetDeltaTime() { return m_DeltaTime; }

    private:
        float                                          m_DeltaTime     = 0;
        std::chrono::high_resolution_clock::time_point m_LastFrameTime = std::chrono::high_resolution_clock::now();
    };
} // namespace SnowLeopardEngine
