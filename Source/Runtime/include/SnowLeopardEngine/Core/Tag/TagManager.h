#pragma once

#include "SnowLeopardEngine/Core/Base/Base.h"

namespace SnowLeopardEngine
{
    using TagValue = uint32_t;

    enum class BuiltinTag
    {
        Untagged   = 0,
        Player     = 1 << 0,
        Enemy      = 1 << 1,
        MainCamera = 1 << 2,
    };
    const TagValue BuiltinTagLastValue = 1 << 3;

    class TagManager
    {
    public:
        static void Init();

        static bool TryGetTagByName(const std::string& name, TagValue& tagValue);
        static bool TryAppendNewTag(const std::string& name);

        static std::unordered_map<std::string, TagValue> s_TagMap;
        static TagValue                                  s_TagValueSeed;
    };
} // namespace SnowLeopardEngine