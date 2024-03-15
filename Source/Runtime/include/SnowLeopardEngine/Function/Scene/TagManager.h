#pragma once

#include "SnowLeopardEngine/Core/Base/Base.h"

namespace SnowLeopardEngine
{
    namespace Tag
    {
#define DECLARE_TAG(tagName) const std::string tagName = #tagName;

        DECLARE_TAG(Untagged)
        DECLARE_TAG(Player)
        DECLARE_TAG(Enemy)
        DECLARE_TAG(Terrain)
        DECLARE_TAG(Sky)
        DECLARE_TAG(SpawnPoint)
    } // namespace Tag

    class TagManager
    {
    public:
        // TODO: Serialization & Deserialization when project loading and saving
        static std::set<std::string> s_TagSet;
        static uint32_t              s_TagIndexSeed;
    };
} // namespace SnowLeopardEngine