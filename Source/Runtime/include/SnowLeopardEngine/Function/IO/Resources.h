#pragma once

#include "SnowLeopardEngine/Core/Base/Base.h"
#include "SnowLeopardEngine/Core/File/FileSystem.h"
#include "SnowLeopardEngine/Core/UUID/CoreUUID.h"

namespace SnowLeopardEngine
{
    class Asset;

    namespace Resources
    {
        // md5 -> asset handle
        extern std::unordered_map<std::string, Ref<Asset>> g_AssetCache;

        // UUID -> asset handle
        extern std::unordered_map<CoreUUID, Ref<Asset>> g_AssetMap;

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

            g_AssetCache[md5]               = outAsset;
            g_AssetMap[outAsset->GetUUID()] = outAsset;
            return true;
        }

        static Ref<Asset> GetAssetByUUID(CoreUUID uuid)
        {
            if (g_AssetMap.count(uuid) == 0)
            {
                return nullptr;
            }
            return g_AssetMap[uuid];
        }
    } // namespace Resources
} // namespace SnowLeopardEngine