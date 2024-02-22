#include "SnowLeopardEngine/Function/Asset/Loaders/TextureLoader.h"
#include "SnowLeopardEngine/Core/Log/LogSystem.h"
#include "SnowLeopardEngine/Engine/EngineContext.h"

#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>

namespace SnowLeopardEngine
{
    bool TextureLoader::LoadTexture2D(const std::filesystem::path& path,
                                      bool                         flip,
                                      Buffer&                      outBuffer,
                                      PixelColorFormat&            outFormat,
                                      uint32_t&                    outWidth,
                                      uint32_t&                    outHeight)
    {
        std::string pathString = path.generic_string();

        stbi_set_flip_vertically_on_load(flip);

        int width, height, channels;
        if (stbi_is_hdr(pathString.c_str()))
        {
            outBuffer.Data =
                reinterpret_cast<std::byte*>(stbi_loadf(pathString.c_str(), &width, &height, &channels, 4));
            outBuffer.Size = width * height * 4 * sizeof(float);
            outFormat      = PixelColorFormat::RGBA32;
        }
        else
        {
            outBuffer.Data = stbi_load(pathString.c_str(), &width, &height, &channels, 4);
            outBuffer.Size = width * height * 4;
            outFormat      = PixelColorFormat::RGBA8;
        }

        if (!outBuffer.Data)
        {
            SNOW_LEOPARD_CORE_ERROR("[TextureLoader] Failed to load texture from: {0}", path.generic_string());
            return false;
        }

        outWidth  = width;
        outHeight = height;

        SNOW_LEOPARD_CORE_INFO("[TextureLoader] Loaded texture from: {0}", path.generic_string());
        return true;
    }
} // namespace SnowLeopardEngine