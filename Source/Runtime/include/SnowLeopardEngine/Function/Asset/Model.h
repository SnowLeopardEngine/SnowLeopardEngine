#pragma once

#include "SnowLeopardEngine/Function/Animation/Animation.h"
#include "SnowLeopardEngine/Function/Rendering/RHI/Texture.h"

namespace SnowLeopardEngine
{
    struct Model
    {
        std::filesystem::path SourcePath;

        MeshGroup Meshes;

        std::unordered_map<std::string, std::vector<Ref<Texture2D>>> Textures;
        std::vector<Ref<Animation>>                                  Animations;
        std::map<std::string, BoneInfo>                              BoneInfoMap;
        int                                                          BoneCounter = 0;
    };
} // namespace SnowLeopardEngine