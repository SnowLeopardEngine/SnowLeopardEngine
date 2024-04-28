#pragma once

#include "SnowLeopardEngine/Function/Rendering/Material.h"
#include "SnowLeopardEngine/Function/Rendering/RenderTypeDef.h"

namespace SnowLeopardEngine
{
    struct Renderable
    {
        MeshItem* Mesh        = nullptr;
        Material* Mat         = nullptr;
        glm::mat4 ModelMatrix = glm::mat4(1.0f);
    };
} // namespace SnowLeopardEngine