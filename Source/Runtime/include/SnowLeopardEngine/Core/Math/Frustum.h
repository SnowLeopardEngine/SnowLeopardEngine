#pragma once

#include "SnowLeopardEngine/Core/Base/Base.h"
#include "SnowLeopardEngine/Core/Math/AABB.h"
#include "SnowLeopardEngine/Core/Math/Plane.h"

namespace SnowLeopardEngine
{
    class Frustum final
    {
    public:
        Frustum() = default;
        explicit Frustum(const glm::mat4&);
        Frustum(const Frustum&)     = delete;
        Frustum(Frustum&&) noexcept = default;
        ~Frustum()                  = default;

        Frustum& operator=(const Frustum&)     = delete;
        Frustum& operator=(Frustum&&) noexcept = default;

        void Update(const glm::mat4& m);

        bool TestPoint(const glm::vec3&) const;
        bool TestAABB(const AABB&) const;

    private:
        std::array<Plane, 6> m_Planes;
    };
} // namespace SnowLeopardEngine