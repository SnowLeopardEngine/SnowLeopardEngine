#pragma once

#include "SnowLeopardEngine/Core/Base/Base.h"

namespace SnowLeopardEngine
{
    namespace Random
    {
        static float GetRandomFloat()
        {
            std::random_device                    rd;
            std::mt19937                          gen(rd());
            std::uniform_real_distribution<float> dis(0.0f, 1.0f);
            return dis(gen);
        }

        static float GetRandomFloatRanged(float min, float max)
        {
            float random = GetRandomFloat();
            return min + random * (max - min);
        }
    } // namespace Random
} // namespace SnowLeopardEngine