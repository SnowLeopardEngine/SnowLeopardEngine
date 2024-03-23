#pragma once

#include "SnowLeopardEngine/Core/Base/Base.h"

#include "entt/entity/fwd.hpp"

namespace SnowLeopardEngine
{
    struct InstancingBatchGroup
    {
        std::vector<entt::entity> Entities;
        std::string               ShaderName;
        std::string               MeshItemName;
    };
} // namespace SnowLeopardEngine