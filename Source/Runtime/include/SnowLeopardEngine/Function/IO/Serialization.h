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

namespace std
{
    namespace filesystem
    {
        template<class Archive>
        void save(Archive& archive, const path& p)
        {
            std::string pathStr = p.generic_string();
            archive(pathStr);
        }

        template<class Archive>
        void load(Archive& archive, path& p)
        {
            std::string pathStr;
            archive(pathStr);
            p = pathStr;
        }
    } // namespace filesystem
} // namespace std

namespace SnowLeopardEngine
{
    class LogicScene;
    class Project;

    namespace IO
    {
        bool Serialize(LogicScene* scene, const std::filesystem::path& dstPath);
        bool Deserialize(LogicScene* scene, const std::filesystem::path& srcPath);

        bool Serialize(Project* project, const std::filesystem::path& dstPath);
        bool Deserialize(Project* project, const std::filesystem::path& srcPath);
    } // namespace IO
} // namespace SnowLeopardEngine