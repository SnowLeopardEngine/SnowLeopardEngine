#pragma once

#include "SnowLeopardEngine/Function/Asset/Asset.h"
#include "SnowLeopardEngine/Function/Rendering/DzMaterial/DzMaterial.h"

namespace SnowLeopardEngine
{
    class DzMaterialAsset : public Asset
    {
    public:
        DzMaterialAsset(const std::filesystem::path& path, const CoreUUID& uuid, Ref<DzMaterial> handle) :
            Asset(path, uuid), m_Handle(handle)
        {}

        Ref<DzMaterial> GetHandle() const { return m_Handle; }

        static bool Load(const std::filesystem::path& path, Ref<DzMaterialAsset>& outAsset);

    private:
        Ref<DzMaterial> m_Handle = nullptr;
    };
} // namespace SnowLeopardEngine