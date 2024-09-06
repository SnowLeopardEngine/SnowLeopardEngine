#pragma once

#include <fg/FrameGraphResource.hpp>

namespace SnowLeopardEngine
{
    struct ShadowData
    {
        FrameGraphResource CascadedUniformBuffer;
        FrameGraphResource CascadedShadowMaps;
    };
} // namespace SnowLeopardEngine