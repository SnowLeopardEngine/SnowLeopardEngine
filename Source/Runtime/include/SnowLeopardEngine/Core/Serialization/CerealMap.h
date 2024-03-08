#pragma once

#include "SnowLeopardEngine/Core/Base/Base.h"

#include <cereal/cereal.hpp>
#include <cereal/types/map.hpp>
#include <cereal/types/string.hpp>

namespace cereal
{
    template<class Archive,
             class C,
             class A,
             traits::EnableIf<traits::is_text_archive<Archive>::value> = traits::sfinae>
    inline void save(Archive& ar, std::map<std::string, std::string, C, A> const& map)
    {
        for (const auto& i : map)
            ar(cereal::make_nvp(i.first, i.second));
    }

    template<class Archive,
             class C,
             class A,
             traits::EnableIf<traits::is_text_archive<Archive>::value> = traits::sfinae>
    inline void load(Archive& ar, std::map<std::string, std::string, C, A>& map)
    {
        map.clear();

        auto hint = map.begin();
        while (true)
        {
            const auto namePtr = ar.getNodeName();

            if (!namePtr)
                break;

            std::string key = namePtr;
            std::string value;
            ar(value);
            hint = map.emplace_hint(hint, std::move(key), std::move(value));
        }
    }
} // namespace cereal