#pragma once

#include "SnowLeopardEngine/Core/Math/Math.h"

#include <fg/Fwd.hpp>

namespace SnowLeopardEngine
{
    struct FrameUniform
    {
        float ElapsedTime;

        alignas(16) glm::mat4 ViewMatrix {1.0};
        alignas(16) glm::mat4 InversedViewMatrix {1.0};
        alignas(16) glm::mat4 ProjectionMatrix {1.0};
        alignas(16) glm::mat4 InversedProjectionMatrix {1.0};

        alignas(16) glm::vec3 ViewPos;
    };

    void uploadFrameUniform(FrameGraph&, FrameGraphBlackboard&, const FrameUniform&);
} // namespace SnowLeopardEngine