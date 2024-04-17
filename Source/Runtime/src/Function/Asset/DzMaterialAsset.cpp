#include "SnowLeopardEngine/Function/Asset/DzMaterialAsset.h"
#include "SnowLeopardEngine/Function/IO/Resources.h"

namespace SnowLeopardEngine
{
    bool DzMaterialAsset::Load(const std::filesystem::path& path, Ref<DzMaterialAsset>& outAsset)
    {
        auto uuid = Resources::GetAssetUUIDByPath(path);
        if (uuid.is_nil())
        {
            uuid = CoreUUIDHelper::CreateStandardUUID();
        }

        auto handle = DzMaterial::LoadFromPath(path);
        if (handle == nullptr)
        {
            outAsset = nullptr;
            return false;
        }

        outAsset = CreateRef<DzMaterialAsset>(path, uuid, handle);

        return true;
    }
} // namespace SnowLeopardEngine