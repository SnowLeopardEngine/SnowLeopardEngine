#pragma once

#include "SnowLeopardEngine/Function/Rendering/RHI/Texture.h"
#include "SnowLeopardEngine/Function/Rendering/RenderTypeDef.h"

namespace SnowLeopardEngine
{
    // NOLINTBEGIN
    class FrameGraphTexture2D
    {
    public:
        struct Desc
        {
            uint32_t         Width  = 1;
            uint32_t         Height = 1;
            PixelColorFormat Format = PixelColorFormat::RGBA8;

            TextureConfig Config;
        };

        void create(const Desc& desc, void* allocator);
        void destroy(const Desc& desc, void* allocator);

    private:
        Ref<Texture2D> m_Texture = nullptr;
    };

    class FrameGraphCubemap
    {
    public:
        struct Desc
        {
            uint32_t         Width  = 1;
            uint32_t         Height = 1;
            PixelColorFormat Format = PixelColorFormat::RGBA8;

            TextureConfig Config;
        };

        void create(const Desc& desc, void* allocator);
        void destroy(const Desc& desc, void* allocator);

    private:
        Ref<Cubemap> m_Cubemap = nullptr;
    };
    // NOLINTEND
} // namespace SnowLeopardEngine