#pragma once

#include "SnowLeopardEngine/Core/Base/Base.h"
#include "SnowLeopardEngine/Core/File/FileSystem.h"
#include "SnowLeopardEngine/Core/UUID/CoreUUID.h"
#include "SnowLeopardEngine/Function/Asset/Asset.h"
#include "SnowLeopardEngine/Function/Project/ProjectTypeDef.h"

namespace SnowLeopardEngine
{
    class Asset;
    class Project;

    namespace Resources
    {
        // md5 -> asset handle
        extern std::unordered_map<std::string, Ref<Asset>> g_AssetCache;

        // UUID -> asset handle
        extern std::unordered_map<CoreUUID, Ref<Asset>> g_AssetMap;

        extern std::vector<AssetMetaData> g_AssetMetaDatas;

        static Ref<Asset> GetAssetByUUID(CoreUUID uuid)
        {
            if (g_AssetMap.count(uuid) == 0)
            {
                return nullptr;
            }
            return g_AssetMap[uuid];
        }

        static CoreUUID GetAssetUUIDByPath(const std::filesystem::path& path)
        {
            CoreUUID ret = {};

            for (const auto& metaData : g_AssetMetaDatas)
            {
                if (metaData.Path == path)
                {
                    ret = metaData.ID;
                    break;
                }
            }

            return ret;
        }

        void OnProjectLoad(Project* project);

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

            auto uuid = GetAssetUUIDByPath(path);
            if (uuid.is_nil())
            {
                AssetMetaData metaData = {};
                metaData.ID            = outAsset->GetUUID();
                metaData.MD5           = md5;
                metaData.Path          = path.generic_string();
                g_AssetMetaDatas.emplace_back(metaData);
            }

            return true;
        }

        static void ImportAssets(const std::filesystem::path&    assetRootPath,
                                 const std::vector<std::string>& assetIgnoreTokens)
        {
            for (const auto& entry : std::filesystem::directory_iterator(assetRootPath))
            {
                if (entry.is_regular_file())
                {
                    bool skip = false;
                    for (const auto& ignoreToken : assetIgnoreTokens)
                    {
                        const auto& entryPathStr = entry.path().generic_string();
                        if (entryPathStr.find(ignoreToken) != entryPathStr.npos)
                        {
                            skip = true;
                            break;
                        }
                    }

                    if (skip)
                        continue;

                    std::string extension = entry.path().extension().string();
                    AssetType   assetType = ExtensionsToType[extension];

                    // Import assets

                    // TODO: Import other assets
                }
                else if (entry.is_directory())
                {
                    ImportAssets(entry.path(), assetIgnoreTokens);
                }
            }
        }
    } // namespace Resources
} // namespace SnowLeopardEngine