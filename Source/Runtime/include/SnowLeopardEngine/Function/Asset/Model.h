#pragma once

#include "SnowLeopardEngine/Function/Animation/Animation.h"
#include "SnowLeopardEngine/Function/Rendering/RenderTypeDef.h"

namespace SnowLeopardEngine
{
    struct Model
    {
        std::filesystem::path SourcePath;

        MeshGroup Meshes;

        std::vector<Ref<Animation>>     Animations;
        std::map<std::string, BoneInfo> BoneInfoMap;
        int                             BoneCounter = 0;
    };
} // namespace SnowLeopardEngine