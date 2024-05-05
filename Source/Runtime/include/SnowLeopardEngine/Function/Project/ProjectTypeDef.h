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
        std::string MD5;

        // NOLINTBEGIN
        template<class Archive>
        void serialize(Archive& archive)
        {
            archive(CEREAL_NVP(ID), CEREAL_NVP(Path), CEREAL_NVP(MD5));
        }
        // NOLINTEND
    };

    struct DzProject
    {
        std::string                Name;
        std::vector<std::string>   AssetIgnoreTokens;
        std::vector<AssetMetaData> AssetMetaDatas;

        // NOLINTBEGIN
        template<class Archive>
        void serialize(Archive& archive)
        {
            archive(CEREAL_NVP(Name), CEREAL_NVP(AssetIgnoreTokens), CEREAL_NVP(AssetMetaDatas));
        }
        // NOLINTEND
    };
} // namespace SnowLeopardEngine