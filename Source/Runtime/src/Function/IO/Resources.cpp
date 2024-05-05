#include "SnowLeopardEngine/Function/IO/Resources.h"
#include "SnowLeopardEngine/Function/Project/Project.h"

namespace SnowLeopardEngine
{
    namespace Resources
    {
        std::unordered_map<std::string, Ref<Asset>> g_AssetCache;
        std::unordered_map<CoreUUID, Ref<Asset>>    g_AssetMap;
        std::vector<AssetMetaData>                  g_AssetMetaDatas;

        void OnProjectLoad(Project* project)
        {
            auto metaDatas = project->GetInfo().AssetMetaDatas;
            if (!metaDatas.empty())
            {
                g_AssetMetaDatas = metaDatas;
            }
        }
    } // namespace Resources
} // namespace SnowLeopardEngine