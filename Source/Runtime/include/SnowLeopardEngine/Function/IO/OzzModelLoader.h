#pragma once

#include "SnowLeopardEngine/Function/Asset/Model.h"

namespace SnowLeopardEngine
{
    struct OzzModelLoadConfig
    {
        std::filesystem::path              OzzMeshPath;
        std::filesystem::path              OzzSkeletonPath;
        std::vector<std::filesystem::path> OzzAnimationPaths;
    };

    class OzzModelLoader
    {
    public:
        static bool Load(const OzzModelLoadConfig& config, Model* model);

    private:
        static bool LoadMesh(const std::filesystem::path& path, Model* model, bool& skinned);
        static bool LoadAnimations(const std::filesystem::path&              skeletonPath,
                                   const std::vector<std::filesystem::path>& animationPaths,
                                   Model*                                    model);
    };
} // namespace SnowLeopardEngine