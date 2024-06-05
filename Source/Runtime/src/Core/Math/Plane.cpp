#include "SnowLeopardEngine/Core/Math/Plane.h"

#include "glm/geometric.hpp"

namespace SnowLeopardEngine
{
    void Plane::Normalize()
    {
        const auto magnitude = glm::length(Normal);
        Normal /= magnitude;
        Distance /= magnitude;
    }

    float Plane::DistanceTo(const glm::vec3& p) const { return glm::dot(Normal, p) + Distance; }
} // namespace SnowLeopardEngine