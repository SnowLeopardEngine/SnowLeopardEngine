#include "SnowLeopardEngine/Function/Asset/TextureAsset.h"

namespace SnowLeopardEngine
{
    bool Texture2DAsset::Load(const std::filesystem::path& path, Ref<Texture2DAsset>& outAsset, bool flip)
    {
        auto                 uuid = CoreUUIDHelper::CreateStandardUUID();
        TextureLoadingOutput output;
        if (!TextureLoader::LoadTexture2D(path, flip, output))
        {
            outAsset = nullptr;
            return false;
        }

        outAsset = CreateRef<Texture2DAsset>(path, uuid, output);
        return true;
    }
} // namespace SnowLeopardEngine