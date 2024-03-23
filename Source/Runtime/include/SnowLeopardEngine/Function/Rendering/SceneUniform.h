#pragma once

#include "SnowLeopardEngine/Core/Math/Math.h"

namespace SnowLeopardEngine
{
    struct SceneUniform
    {
        glm::mat4 ViewMatrix;
        glm::mat4 ProjectionMatrix;
        glm::vec3 ViewPos;
        glm::mat4 LightSpaceMatrix;
        float     DirectionalLightIntensity = 0.8f;
        glm::vec3 DirectionalLightDirection;
        glm::vec3 DirectionalLightColor;
        float     ElapsedTime = 0;
    };
} // namespace SnowLeopardEngine