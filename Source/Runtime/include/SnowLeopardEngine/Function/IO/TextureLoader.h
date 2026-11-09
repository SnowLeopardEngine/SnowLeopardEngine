#pragma once

#include "SnowLeopardEngine/Function/Rendering/RHI/Resources.h"

#include <filesystem>

namespace SnowLeopardEngine
{
    namespace IO
    {
        // A texture by its file path. The imported (cooked) texture when there is one (AssetManager), else the image
        // file is decoded into RGBA8 with a CPU mip chain (a counted fallback). Rows are top to bottom, as in the file:
        // meshes are imported with v = 0 at the top, and the UI flips its own v. Cached by path; ReleaseAllTextures
        // (render system shutdown) frees them.
        RHI::Texture* LoadTexture(const std::filesystem::path& texturePath);
        // An equirectangular .hdr environment: the cooked RGBA16F texture, else the file as RGBA32F (one level).
        RHI::Texture* LoadHDR(const std::filesystem::path& texturePath);

        void ReleaseAllTextures();
    } // namespace IO
} // namespace SnowLeopardEngine
