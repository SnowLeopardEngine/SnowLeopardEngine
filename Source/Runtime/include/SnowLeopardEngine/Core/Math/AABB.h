#pragma once

#include "SnowLeopardEngine/Function/Rendering/RenderTypeDef.h"

#include "glm/mat4x4.hpp"
#include "glm/vec3.hpp"

namespace SnowLeopardEngine
{
    struct AABB
    {
        glm::vec3 Min, Max;

        glm::vec3 GetExtent() const;
        glm::vec3 GetCenter() const;
        float     GetRadius() const;

        void Merge(const AABB& other);

        AABB Transform(const glm::mat4&) const;

        static AABB Build(const std::vector<MeshVertexData>& vertices);

        // clang-format off
        auto operator<=> (const AABB&) const = delete;
        // clang-format on
    };
} // namespace SnowLeopardEngine