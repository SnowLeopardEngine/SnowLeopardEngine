#include "SnowLeopardEngine/Function/Scene/TagManager.h"

namespace SnowLeopardEngine
{
    std::vector<std::string> TagManager::s_Tags =
        {Tag::Untagged, Tag::Player, Tag::Enemy, Tag::Terrain, Tag::Sky, Tag::SpawnPoint};
    uint32_t TagManager::s_TagIndexSeed = s_Tags.size();
} // namespace SnowLeopardEngine