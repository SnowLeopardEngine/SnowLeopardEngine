#include "SnowLeopardEngine/Core/Math/Frustum.h"

namespace SnowLeopardEngine
{
    enum FrustumSide
    {
        Right  = 0,
        Left   = 1,
        Bottom = 2,
        Top    = 3,
        Near   = 4, // Back
        Far    = 5  // Front
    };

    enum class IntersectionResult
    {
        Outside,
        Intersect,
        Inside
    };

    Frustum::Frustum(const glm::mat4& m) { Update(m); }

    void Frustum::Update(const glm::mat4& m)
    {
        m_Planes[FrustumSide::Left] = Plane {
            .Normal   = {m[0][3] + m[0][0], m[1][3] + m[1][0], m[2][3] + m[2][0]},
            .Distance = m[3][3] + m[3][0],
        };
        m_Planes[FrustumSide::Right] = Plane {
            .Normal   = {m[0][3] - m[0][0], m[1][3] - m[1][0], m[2][3] - m[2][0]},
            .Distance = m[3][3] - m[3][0],
        };
        m_Planes[FrustumSide::Bottom] = Plane {
            .Normal   = {m[0][3] + m[0][1], m[1][3] + m[1][1], m[2][3] + m[2][1]},
            .Distance = m[3][3] + m[3][1],
        };
        m_Planes[FrustumSide::Top] = Plane {
            .Normal   = {m[0][3] - m[0][1], m[1][3] - m[1][1], m[2][3] - m[2][1]},
            .Distance = m[3][3] - m[3][1],
        };
        m_Planes[FrustumSide::Near] = Plane {
            .Normal   = {m[0][3] + m[0][2], m[1][3] + m[1][2], m[2][3] + m[2][2]},
            .Distance = m[3][3] + m[3][2],
        };
        m_Planes[FrustumSide::Far] = Plane {
            .Normal   = {m[0][3] - m[0][2], m[1][3] - m[1][2], m[2][3] - m[2][2]},
            .Distance = m[3][3] - m[3][2],
        };

        for (auto& plane : m_Planes)
            plane.Normalize();
    }

    bool Frustum::TestPoint(const glm::vec3& point) const
    {
        for (const auto& plane : m_Planes)
            if (plane.DistanceTo(point) < 0)
                return false; // Outside

        return true; // Inside
    }

    bool Frustum::TestAABB(const AABB& aabb) const
    {
        auto result {IntersectionResult::Inside};
        for (const auto& plane : m_Planes)
        {
            auto pv = aabb.Min;
            auto nv = aabb.Max;
            if (plane.Normal.x >= 0)
            {
                pv.x = aabb.Max.x;
                nv.x = aabb.Min.x;
            }
            if (plane.Normal.y >= 0)
            {
                pv.y = aabb.Max.y;
                nv.y = aabb.Min.y;
            }
            if (plane.Normal.z >= 0)
            {
                pv.z = aabb.Max.z;
                nv.z = aabb.Min.z;
            }
            if (plane.DistanceTo(pv) < 0)
                return false;
            if (plane.DistanceTo(nv) < 0)
                result = IntersectionResult::Intersect;
        }
        return result != IntersectionResult::Outside;
    }
} // namespace SnowLeopardEngine