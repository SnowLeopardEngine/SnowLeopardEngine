#pragma once

#include "SnowLeopardEngine/Core/Base/Base.h"
#include "SnowLeopardEngine/Function/Asset/Asset.h"
#include "SnowLeopardEngine/Function/IO/Serialization.h"

#include <cereal/archives/json.hpp>
#include <cereal/types/vector.hpp>

namespace SnowLeopardEngine
{
    struct AssetMetaData
    {
        CoreUUID    ID;
        std::string Path;

        // NOLINTBEGIN
        template<class Archive>
        void serialize(Archive& archive)
        {
            archive(CEREAL_NVP(ID), CEREAL_NVP(Path));
        }
        // NOLINTEND
    };

    // UUID -> Metadata
    using AssetMap = std::map<CoreUUID, AssetMetaData>;

    // MD5 -> UUID
    using AssetContentMap = std::map<std::string, CoreUUID>;

    struct ProjectInfo
    {
        std::string           Name;
        std::filesystem::path RootPath;

        AssetMap        AssetMapInfo;
        AssetContentMap AssetContentInfo;

        // NOLINTBEGIN
        template<class Archive>
        void serialize(Archive& archive)
        {
            archive(CEREAL_NVP(Name), CEREAL_NVP(RootPath), CEREAL_NVP(AssetMapInfo), CEREAL_NVP(AssetContentInfo));
        }
        // NOLINTEND
    };
} // namespace SnowLeopardEngine