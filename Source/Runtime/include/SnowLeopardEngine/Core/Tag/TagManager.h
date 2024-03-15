#pragma once

#include "SnowLeopardEngine/Core/Base/Base.h"

namespace SnowLeopardEngine
{
    using TagValue = uint32_t;

    enum class BuiltinTag : TagValue
    {
        Untagged   = 0,
        Player     = 1 << 0,
        Enemy      = 1 << 1,
        MainCamera = 1 << 2,
        Terrain    = 1 << 3,
        Sky        = 1 << 4,
    };
    const TagValue BuiltinTagNextValue = 1 << 5;

    class TagManager
    {
    public:
        static void Init();

        static bool TryGetTagByName(const std::string& name, TagValue& tagValue);
        static bool TryAppendNewTag(const std::string& name, TagValue& tagValue);

        static std::unordered_map<std::string, TagValue> s_TagMap;
        static TagValue                                  s_TagValueSeed;
    };
} // namespace SnowLeopardEngine