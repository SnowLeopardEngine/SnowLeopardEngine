#pragma once

#include "SnowLeopardEngine/Core/Math/Math.h"

#include <fg/Fwd.hpp>

namespace SnowLeopardEngine
{
    const uint32_t kMaxPointLights = 4;

    struct GPUDirectionalLight
    {
        float Intensity = 0;

        alignas(16) glm::mat4 LightSpaceMatrix {1.0};
        alignas(16) glm::vec3 Direction;
        alignas(16) glm::vec3 Color;
    };

    struct GPUPointLight
    {
        float Constant  = 0;
        float Linear    = 0;
        float Quadratic = 0;

        alignas(16) glm::mat4 LightSpaceMatrix {1.0};
        alignas(16) glm::vec3 Position;
        alignas(16) glm::vec3 Color;
    };

    struct LightUniform
    {
        uint32_t            NumPointLights = 0;
        GPUDirectionalLight DirectionalLight;
        GPUPointLight       PointLights[kMaxPointLights];
    };

    void uploadLightUniform(FrameGraph&, FrameGraphBlackboard&, const LightUniform&);
} // namespace SnowLeopardEngine