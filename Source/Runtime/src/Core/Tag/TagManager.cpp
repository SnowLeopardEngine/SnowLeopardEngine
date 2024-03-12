#include "SnowLeopardEngine/Core/Tag/TagManager.h"
#include "magic_enum.hpp"

namespace SnowLeopardEngine
{
    std::unordered_map<std::string, TagValue> TagManager::s_TagMap;
    TagValue                                  TagManager::s_TagValueSeed = BuiltinTagLastValue;

    void TagManager::Init()
    {
        // Init built-in tags
        auto builtinEntries = magic_enum::enum_entries<BuiltinTag>();
        for (const auto& [tagValue, tagName] : builtinEntries)
        {
            s_TagMap[std::string(tagName)] = static_cast<uint32_t>(tagValue);
        }
    }

    bool TagManager::TryGetTagByName(const std::string& name, TagValue& tagValue)
    {
        if (s_TagMap.count(name) == 0)
        {
            return false;
        }

        tagValue = s_TagMap[name];
        return true;
    }

    bool TagManager::TryAppendNewTag(const std::string& name)
    {
        if (s_TagMap.count(name) > 0)
        {
            return false;
        }

        s_TagMap[name] = s_TagValueSeed++;
        return true;
    }
} // namespace SnowLeopardEngine