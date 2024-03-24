#include "SnowLeopardEngine/Function/IO/Resources.h"

namespace SnowLeopardEngine
{
    namespace Resources
    {
        std::unordered_map<std::string, Ref<Asset>> g_AssetCache;
    }
} // namespace SnowLeopardEngine