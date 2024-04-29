#pragma once

#include "SnowLeopardEngine/Core/Math/Math.h"

#include <fg/Fwd.hpp>

namespace SnowLeopardEngine
{
    struct FrameUniform
    {
        float ElapsedTime;

        alignas(16) glm::mat4 ViewMatrix;
        alignas(16) glm::mat4 ProjectionMatrix;
        alignas(16) glm::vec3 ViewPos;
    };

    void uploadFrameUniform(FrameGraph&, FrameGraphBlackboard&, const FrameUniform&);
} // namespace SnowLeopardEngine