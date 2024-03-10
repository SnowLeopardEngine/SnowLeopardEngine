#include "SnowLeopardEngine/Function/Animation/AnimationClip.h"
#include "SnowLeopardEngine/Engine/EngineContext.h"

#include "ozz/base/io/archive.h"
#include "ozz/base/io/stream.h"

namespace SnowLeopardEngine
{
    bool AnimationClip::LoadFromOzzFiles(const std::string& skeletonPath, const std::string& animationPath)
    {
        if (!LoadSkeletonData(skeletonPath))
        {
            SNOW_LEOPARD_CORE_ERROR("Failed to load ozz format skeleton data from {0}", skeletonPath);
            return false;
        }

        if (!LoadAnimationData(animationPath))
        {
            SNOW_LEOPARD_CORE_ERROR("Failed to load ozz format animation data from {0}", animationPath);
            return false;
        }

        // Skeleton and animation needs to match.
        if (Skeleton.num_joints() != Animation.num_tracks())
        {
            SNOW_LEOPARD_CORE_ERROR("Skeleton and Animation data mismatch!");
            return false;
        }

        // Allocate runtime buffers
        const int numSoaJoints = Skeleton.num_soa_joints();
        const int numJoints    = Skeleton.num_joints();
        LocalTransforms.resize(numSoaJoints);
        ModelMatrices.resize(numJoints);

        // Allocates a context that matches animation requirements.
        SamplingJobContext.Resize(numJoints);

        return true;
    }

    bool AnimationClip::LoadSkeletonData(const std::string& fileName)
    {
        ozz::io::File skeletonFile(fileName.c_str(), "rb");
        if (!skeletonFile.opened())
        {
            return false;
        }

        ozz::io::IArchive archive(&skeletonFile);

        if (!archive.TestTag<ozz::animation::Skeleton>())
        {
            return false;
        }

        archive >> Skeleton;

        return true;
    }

    bool AnimationClip::LoadAnimationData(const std::string& fileName)
    {
        ozz::io::File animationFile(fileName.c_str(), "rb");
        if (!animationFile.opened())
        {
            return false;
        }

        ozz::io::IArchive archive(&animationFile);

        if (!archive.TestTag<ozz::animation::Animation>())
        {
            return false;
        }

        archive >> Animation;

        return true;
    }
} // namespace SnowLeopardEngine