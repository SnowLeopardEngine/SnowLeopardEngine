#pragma once

#include "SnowLeopardEngine/Core/Containers/Buffer.h"
#include "SnowLeopardEngine/Function/Rendering/RenderTypeDef.h"

namespace SnowLeopardEngine
{
    class TextureLoader
    {
    public:
        static bool LoadTexture2D(const std::filesystem::path& path,
                                  bool                         flip,
                                  Buffer&                      outBuffer,
                                  PixelColorFormat&            outFormat,
                                  uint32_t&                    outWidth,
                                  uint32_t&                    outHeight);
    };
} // namespace SnowLeopardEngine