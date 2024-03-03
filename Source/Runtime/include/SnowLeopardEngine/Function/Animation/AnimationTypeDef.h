#pragma once

#include "SnowLeopardEngine/Core/Base/Base.h"
#include "SnowLeopardEngine/Core/Math/Math.h"

namespace SnowLeopardEngine
{
    const uint32_t MaxBoneInfluence = 4;

    struct BoneInfo
    {
        // id is index in finalBoneMatrices
        int Id = -1;

        // offset matrix transforms vertex from model space to bone space
        glm::mat4 Offset;
    };

    struct AnimationClip
    {

    };

    struct PerVertexAnimationAttributes
    {
        // bone indexes which will influence this vertex
        int BoneIDs[MaxBoneInfluence];

        // weights from each bone
        float Weights[MaxBoneInfluence];
    };

    struct PerModelAnimationData
    {
        std::vector<AnimationClip>      AnimationClips;
        std::map<std::string, BoneInfo> BoneInfoMap;
        int                             BoneCounter = 0;
    };
} // namespace SnowLeopardEngine