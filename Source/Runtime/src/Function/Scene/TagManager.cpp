#include "SnowLeopardEngine/Function/Scene/TagManager.h"

namespace SnowLeopardEngine
{
    std::set<std::string> TagManager::s_TagSet =
        {Tag::Untagged, Tag::Player, Tag::Enemy, Tag::Terrain, Tag::Sky, Tag::SpawnPoint};
    uint32_t TagManager::s_TagIndexSeed = s_TagSet.size();
} // namespace SnowLeopardEngine