#pragma once

#include "SnowLeopardEngine/Core/Math/Math.h"
#include "SnowLeopardEngine/Function/Scene/Entity.h"

namespace SnowLeopardEngine
{
    struct ShadowCascade
    {
        float     SplitDepth;
        glm::mat4 LightSpaceMatrices;
    };

    std::vector<ShadowCascade> buildCascades(Entity           cameraEntity,
                                             const glm::vec3& lightDirection,
                                             uint32_t         numCascades,
                                             float            lambda,
                                             uint32_t         shadowMapSize);
} // namespace SnowLeopardEngine