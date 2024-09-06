#pragma once

#include "glm/vec3.hpp"

namespace SnowLeopardEngine
{
    struct Plane
    {
        glm::vec3 Normal {0.0f};
        float     Distance {0.0f};

        void  Normalize();
        float DistanceTo(const glm::vec3&) const;
    };
} // namespace SnowLeopardEngine::Math