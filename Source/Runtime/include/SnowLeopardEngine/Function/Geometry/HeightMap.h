#pragma once

#include "SnowLeopardEngine/Core/Base/Base.h"
#include <cstdint>

namespace SnowLeopardEngine
{
    struct HeightMap
    {
        uint32_t Width;
        uint32_t Height;

        std::vector<std::vector<float>> Data;

        HeightMap()                 = default;
        HeightMap(const HeightMap&) = default;

        void SetData(const std::vector<std::vector<float>>& data)
        {
            Data   = data;
            Width  = data[0].size();
            Height = data.size();
        }

        void  Set(uint32_t column, uint32_t row, float value) { Data[row][column] = value; }
        float Get(uint32_t column, uint32_t row) const { return Data[row][column]; }
    };
} // namespace SnowLeopardEngine