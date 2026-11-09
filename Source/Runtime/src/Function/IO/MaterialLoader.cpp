#include "SnowLeopardEngine/Function/IO/MaterialLoader.h"
#include "SnowLeopardEngine/Engine/EngineContext.h"
#include "SnowLeopardEngine/Function/Asset/AssetManager.h"
#include "SnowLeopardEngine/Function/IO/Serialization.h"
#include "SnowLeopardEngine/Function/Rendering/Material.h"

namespace SnowLeopardEngine
{
    namespace IO
    {
        std::unordered_map<std::string, Scope<Material>> g_MaterialCache;

        Material* Load(const std::filesystem::path& materialPath)
        {
            if (const auto id = AssetManager::FindBySource(materialPath))
            {
                if (auto* material = AssetManager::LoadMaterial(*id))
                {
                    return material;
                }
            }

            auto pathStr = materialPath.generic_string();

            if (auto it = g_MaterialCache.find(pathStr); it != g_MaterialCache.end())
            {
                return it->second.get();
            }
            AssetManager::ReportFallback(materialPath);

            auto material = CreateScope<Material>();

            if (!Deserialize(material.get(), materialPath))
            {
                SNOW_LEOPARD_CORE_ERROR("[MaterialLoader] Failed to deserialize {0}", pathStr);
                return nullptr;
            }

            Material* result         = material.get();
            g_MaterialCache[pathStr] = std::move(material);

            return result;
        }

        void ReleaseAllMaterials() { g_MaterialCache.clear(); }
    } // namespace IO

} // namespace SnowLeopardEngine