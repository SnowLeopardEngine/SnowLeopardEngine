#include "SnowLeopardEngine/Core/Math/AABB.h"

namespace SnowLeopardEngine
{
    glm::vec3 AABB::GetExtent() const { return Max - Min; }
    glm::vec3 AABB::GetCenter() const { return (Max + Min) * 0.5f; }
    float     AABB::GetRadius() const { return glm::length(GetExtent() * 0.5f); }

    // https://dev.theomader.com/transform-bounding-boxes/
    AABB AABB::Transform(const glm::mat4& m) const
    {
        const auto xa = m[0] * Min.x;
        const auto xb = m[0] * Max.x;

        const auto ya = m[1] * Min.y;
        const auto yb = m[1] * Max.y;

        const auto za = m[2] * Min.z;
        const auto zb = m[2] * Max.z;

        return {
            .Min = {glm::min(xa, xb) + glm::min(ya, yb) + glm::min(za, zb) + m[3]},
            .Max = {glm::max(xa, xb) + glm::max(ya, yb) + glm::max(za, zb) + m[3]},
        };
    }
} // namespace SnowLeopardEngine