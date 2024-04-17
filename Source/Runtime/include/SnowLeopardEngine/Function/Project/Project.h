#pragma once

#include "SnowLeopardEngine/Function/IO/Resources.h"
#include "SnowLeopardEngine/Function/IO/Serialization.h"
#include "SnowLeopardEngine/Function/Project/ProjectTypeDef.h"

namespace SnowLeopardEngine
{
    class Project
    {
    public:
        void             SetInfo(const DzProject& info) { m_Info = info; }
        const DzProject& GetInfo() const { return m_Info; }

        void                         SetPath(const std::filesystem::path& path) { m_Path = path; }
        const std::filesystem::path& GetPath() const { return m_Path; }

        void               SetName(const std::string& name) { m_Info.Name = name; }
        const std::string& GetName() const { return m_Info.Name; }

        void LoadAssets();

        void Save() { IO::Serialize(this, m_Path); }

        void RecordAssetInfo(const AssetMetaData& metaData) { m_Info.AssetMetaDatas.emplace_back(metaData); }

        AssetMetaData* GetAssetMetaData(CoreUUID uuid)
        {
            for (auto& metaData : m_Info.AssetMetaDatas)
            {
                if (metaData.ID == uuid)
                {
                    return &metaData;
                }
            }

            return nullptr;
        }

        CoreUUID GetAssetUUID(const std::string& md5)
        {
            CoreUUID ret = {};

            for (auto& metaData : m_Info.AssetMetaDatas)
            {
                if (metaData.MD5 == md5)
                {
                    ret = metaData.ID;
                    break;
                }
            }

            return ret;
        }

    private:
        DzProject             m_Info;
        std::filesystem::path m_Path;
    };
} // namespace SnowLeopardEngine