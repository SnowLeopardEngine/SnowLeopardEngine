#include "SnowLeopardEngine/Core/Math/AABB.h"

namespace SnowLeopardEngine
{
    glm::vec3 AABB::GetExtent() const { return Max - Min; }
    glm::vec3 AABB::GetCenter() const { return (Max + Min) * 0.5f; }
    float     AABB::GetRadius() const { return glm::length(GetExtent() * 0.5f); }

    void AABB::Merge(const AABB& other)
    {
        Min.x = std::min(Min.x, other.Min.x);
        Min.y = std::min(Min.y, other.Min.y);
        Min.z = std::min(Min.z, other.Min.z);

        Max.x = std::max(Max.x, other.Max.x);
        Max.y = std::max(Max.y, other.Max.y);
        Max.z = std::max(Max.z, other.Max.z);
    }

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

    AABB AABB::Build(const std::vector<MeshVertexData>& vertices)
    {
        float maxFloat = std::numeric_limits<float>().max();
        AABB  aabb     = {.Min = glm::vec3(maxFloat), .Max = glm::vec3(-maxFloat)};

        for (const auto& vertex : vertices)
        {
            aabb.Min.x = std::min(aabb.Min.x, vertex.Position.x);
            aabb.Min.y = std::min(aabb.Min.y, vertex.Position.y);
            aabb.Min.z = std::min(aabb.Min.z, vertex.Position.z);

            aabb.Max.x = std::max(aabb.Max.x, vertex.Position.x);
            aabb.Max.y = std::max(aabb.Max.y, vertex.Position.y);
            aabb.Max.z = std::max(aabb.Max.z, vertex.Position.z);
        }

        return aabb;
    }
} // namespace SnowLeopardEngine