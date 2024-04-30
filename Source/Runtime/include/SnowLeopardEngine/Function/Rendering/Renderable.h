#pragma once

#include "SnowLeopardEngine/Core/Math/AABB.h"
#include "SnowLeopardEngine/Function/Rendering/Material.h"
#include "SnowLeopardEngine/Function/Rendering/RenderTypeDef.h"

namespace SnowLeopardEngine
{
    struct Renderable
    {
        MeshItem* Mesh        = nullptr;
        Material* Mat         = nullptr;
        glm::mat4 ModelMatrix = glm::mat4(1.0f);

        AABB BoundingBox;
    };

    enum class RenderableGroupType
    {
        Default = 0,
        Instancing
    };

    struct RenderableGroup
    {
        RenderableGroupType     GroupType;
        std::vector<Renderable> Renderables;
    };

    using RenderableGroups = std::unordered_map<size_t, RenderableGroup>;
} // namespace SnowLeopardEngine