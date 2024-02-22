#include "SnowLeopardEngine/Function/Rendering/RHI/Texture.h"
#include "SnowLeopardEngine/Platform/OpenGL/GLTexture.h"

namespace SnowLeopardEngine
{
    Ref<Texture2D> Texture2D::Create(const TextureDesc& desc, Buffer* data)
    {
        return CreateRef<GLTexture2D>(desc, data);
    }
} // namespace SnowLeopardEngine