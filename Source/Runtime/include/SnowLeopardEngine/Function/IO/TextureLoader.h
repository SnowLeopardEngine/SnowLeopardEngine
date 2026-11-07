#pragma once

#include "SnowLeopardEngine/Function/Rendering/RHI/Resources.h"

#include <filesystem>

namespace SnowLeopardEngine
{
    namespace IO
    {
        // Loads an image file into an RGBA8 GPU texture with a full mip chain. Textures are cached by path and owned
        // by the cache; ReleaseAllTextures (render system shutdown) frees them.
        RHI::Texture* Load(const std::filesystem::path& texturePath, bool flip = true);
        // Loads a Radiance .hdr (e.g. an equirectangular environment) into an RGBA32F texture, one level, not flipped.
        RHI::Texture* LoadHDR(const std::filesystem::path& texturePath);

        void ReleaseAllTextures();
    } // namespace IO
} // namespace SnowLeopardEngine
