#pragma once

#include "SnowLeopardEngine/Core/Math/Math.h"

#include <fg/Fwd.hpp>

namespace SnowLeopardEngine
{
    struct FrameUniform
    {
        float DirectionalLightIntensity;
        float ElapsedTime;

        alignas(16) glm::mat4 ViewMatrix;
        alignas(16) glm::mat4 ProjectionMatrix;
        alignas(16) glm::mat4 LightSpaceMatrix;

        alignas(16) glm::vec3 ViewPos;
        alignas(16) glm::vec3 DirectionalLightDirection;
        alignas(16) glm::vec3 DirectionalLightColor;
    };

    void uploadFrameUniform(FrameGraph&, FrameGraphBlackboard&, const FrameUniform&);
} // namespace SnowLeopardEngine