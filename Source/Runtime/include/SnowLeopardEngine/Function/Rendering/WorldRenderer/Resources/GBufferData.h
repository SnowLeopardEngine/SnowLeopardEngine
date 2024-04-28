#pragma once

#include <fg/FrameGraphResource.hpp>

namespace SnowLeopardEngine
{
    struct GBufferData
    {
        FrameGraphResource Position;
        FrameGraphResource Normal;
        FrameGraphResource Albedo;
        FrameGraphResource MetallicRoughnessAO;
        FrameGraphResource EntityID;

        FrameGraphResource Depth;
    };
} // namespace SnowLeopardEngine