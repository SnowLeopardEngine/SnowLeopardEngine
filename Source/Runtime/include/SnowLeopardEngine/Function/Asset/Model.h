#pragma once

#include "SnowLeopardEngine/Function/Animation/AnimationClip.h"
#include "SnowLeopardEngine/Function/Rendering/RenderTypeDef.h"

namespace SnowLeopardEngine
{
    struct Model
    {
        std::filesystem::path SourcePath;

        MeshGroup Meshes;

        std::vector<Ref<AnimationClip>> AnimationClips; // ozz
    };
} // namespace SnowLeopardEngine