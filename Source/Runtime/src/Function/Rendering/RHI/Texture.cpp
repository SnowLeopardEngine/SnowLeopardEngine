#include "SnowLeopardEngine/Function/Rendering/RHI/Texture.h"
#include "SnowLeopardEngine/Platform/OpenGL/GLTexture.h"

namespace SnowLeopardEngine
{
    Ref<Texture2D> Texture2D::Create(const TextureDesc& desc, Buffer* data)
    {
        return CreateRef<GLTexture2D>(desc, data);
    }

    Ref<Cubemap> Cubemap::Create(const TextureDesc& desc, std::vector<Buffer*> dataList)
    {
        return CreateRef<GLCubemap>(desc, dataList);
    }
} // namespace SnowLeopardEngine