#pragma once

#include "SnowLeopardEngine/Core/Base/Base.h"
#include "SnowLeopardEngine/Core/File/FileSystem.h"
#include <memory>

namespace SnowLeopardEngine
{
    class Asset;

    namespace Resources
    {
        // md5 -> asset handle
        extern std::unordered_map<std::string, Ref<Asset>> g_AssetCache;

        template<typename TAsset, typename... Args>
        static bool Load(const std::filesystem::path& path, Ref<TAsset>& outAsset, Args&&... args)
        {
            auto md5 = FileSystem::GetMD5(path);
            if (!md5.empty() && g_AssetCache.count(md5) > 0)
            {
                outAsset = std::dynamic_pointer_cast<TAsset>(g_AssetCache[md5]);
                return true;
            }

            bool result = TAsset::Load(path, outAsset, args...);

            if (!result)
            {
                return false;
            }

            g_AssetCache[md5] = outAsset;
            return true;
        }
    } // namespace Resources
} // namespace SnowLeopardEngine