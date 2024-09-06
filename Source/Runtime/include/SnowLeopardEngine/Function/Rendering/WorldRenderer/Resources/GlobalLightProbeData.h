#pragma once

#include <fg/FrameGraphResource.hpp>

namespace SnowLeopardEngine
{
    struct GlobalLightProbeData
    {
        FrameGraphResource Diffuse;
        FrameGraphResource Specular;
    };
} // namespace SnowLeopardEngine