#pragma once

#include "SnowLeopardEngine/Function/Scene/LogicScene.h"

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
    namespace IO
    {
        bool Serialize(LogicScene* scene, const std::filesystem::path& dstPath);
        bool Deserialize(LogicScene* scene, const std::filesystem::path& srcPath);
    } // namespace IO
} // namespace SnowLeopardEngine