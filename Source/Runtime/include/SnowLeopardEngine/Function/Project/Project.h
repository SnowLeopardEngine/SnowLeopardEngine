#pragma once

#include "SnowLeopardEngine/Function/Project/ProjectTypeDef.h"

namespace SnowLeopardEngine
{
    class Project
    {
    public:
        void               SetInfo(const ProjectInfo& info) { m_Info = info; }
        const ProjectInfo& GetInfo() const { return m_Info; }

        void               SetName(const std::string& name) { m_Info.Name = name; }
        const std::string& GetName() const { return m_Info.Name; }

        void                         SetRootPath(const std::filesystem::path& rootPath) { m_Info.RootPath = rootPath; }
        const std::filesystem::path& GetRootPath() const { return m_Info.RootPath; }

        void RecordAssetInfo(CoreUUID uuid, const AssetMetaData& metaData) { m_Info.AssetMapInfo[uuid] = metaData; }

        AssetMetaData* GetAssetMetaData(CoreUUID uuid)
        {
            if (m_Info.AssetMapInfo.count(uuid) > 0)
                return &m_Info.AssetMapInfo[uuid];
            return nullptr;
        }

        CoreUUID GetAssetUUID(const std::string& md5)
        {
            CoreUUID ret = {};

            if (m_Info.AssetContentInfo.count(md5) > 0)
                ret = m_Info.AssetContentInfo[md5];

            return ret;
        }

    private:
        ProjectInfo m_Info;
    };
} // namespace SnowLeopardEngine