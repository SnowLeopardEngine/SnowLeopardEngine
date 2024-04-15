#include "SnowLeopardEngine/Function/IO/Resources.h"

namespace SnowLeopardEngine
{
    namespace Resources
    {
        std::unordered_map<std::string, Ref<Asset>> g_AssetCache;
        std::unordered_map<CoreUUID, Ref<Asset>>    g_AssetMap;
    } // namespace Resources
} // namespace SnowLeopardEngine