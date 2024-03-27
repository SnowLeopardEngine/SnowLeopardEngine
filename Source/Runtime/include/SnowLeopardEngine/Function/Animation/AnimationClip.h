#pragma once

#include "SnowLeopardEngine/Core/Base/Base.h"

#include "ozz/animation/runtime/animation.h"
#include "ozz/animation/runtime/local_to_model_job.h"
#include "ozz/animation/runtime/sampling_job.h"
#include "ozz/animation/runtime/skeleton.h"
#include "ozz/base/containers/vector.h"
#include "ozz/base/maths/simd_math.h"
#include "ozz/base/maths/soa_transform.h"

namespace SnowLeopardEngine
{
    class MeshItem;

    struct AnimationClip
    {
        MeshItem*                            MeshPtr;
        ozz::animation::Skeleton             Skeleton;
        ozz::animation::Animation            Animation;
        ozz::animation::SamplingJob::Context SamplingJobContext;
        ozz::vector<ozz::math::SoaTransform> LocalTransforms;
        ozz::vector<ozz::math::Float4x4>     ModelMatrices;
        ozz::vector<ozz::math::Float4x4>     SkinningMatrices;

        bool LoadFromOzzFiles(const std::string& skeletonPath, const std::string& animationPath);

    private:
        bool LoadSkeletonData(const std::string& fileName);
        bool LoadAnimationData(const std::string& fileName);
    };
} // namespace SnowLeopardEngine