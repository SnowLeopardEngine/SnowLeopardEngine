#include "SnowLeopardEngine/Function/IO/MaterialLoader.h"
#include "SnowLeopardEngine/Engine/EngineContext.h"
#include "SnowLeopardEngine/Function/IO/Serialization.h"
#include "SnowLeopardEngine/Function/Rendering/Material.h"

namespace SnowLeopardEngine
{
    namespace IO
    {
        std::unordered_map<std::string, Material*> g_MaterialCache;

        Material* Load(const std::filesystem::path& materialPath)
        {
            auto pathStr = materialPath.generic_string();

            if (g_MaterialCache.count(pathStr) > 0)
            {
                return g_MaterialCache[pathStr];
            }

            auto* material = new Material();

            if (!Deserialize(material, materialPath))
            {
                SNOW_LEOPARD_CORE_ERROR("[MaterialLoader] Failed to deserialize {0}", pathStr);
                return nullptr;
            }

            g_MaterialCache[pathStr] = material;

            return material;
        }
    } // namespace IO

} // namespace SnowLeopardEngine