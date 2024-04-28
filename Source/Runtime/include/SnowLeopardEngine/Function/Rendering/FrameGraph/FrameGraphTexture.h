#pragma once

#include "SnowLeopardEngine/Function/Rendering/Texture.h"

namespace SnowLeopardEngine
{
    enum class WrapMode
    {
        ClampToEdge = 0,
        ClampToOpaqueBlack,
        ClampToOpaqueWhite
    };

    class FrameGraphTexture
    {
    public:
        struct Desc
        {
            Extent2D    Extent;
            uint32_t    Depth {0};
            uint32_t    NumMipLevels {1};
            uint32_t    Layers {0};
            PixelFormat Format {PixelFormat::Unknown};

            bool        ShadowSampler {false};
            WrapMode    Wrap {WrapMode::ClampToEdge};
            TexelFilter Filter {TexelFilter::Linear};
        };

        // NOLINTBEGIN
        void create(const Desc& desc, void* allocator);
        void destroy(const Desc& desc, void* allocator);
        // NOLINTEND

        Texture* Handle = nullptr;
    };

} // namespace SnowLeopardEngine