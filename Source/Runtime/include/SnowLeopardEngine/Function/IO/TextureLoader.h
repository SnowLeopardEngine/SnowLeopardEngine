#pragma once

#include "SnowLeopardEngine/Function/Rendering/RHI/Resources.h"

#include <filesystem>

namespace SnowLeopardEngine
{
    namespace IO
    {
        // Loads an image file into an RGBA8 GPU texture. Textures are cached by path and owned by the cache;
        // ReleaseAllTextures (render system shutdown) frees them.
        RHI::Texture* Load(const std::filesystem::path& texturePath, bool flip = true);

        void ReleaseAllTextures();
    } // namespace IO
} // namespace SnowLeopardEngine
