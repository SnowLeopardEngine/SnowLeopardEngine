#pragma once

#include "SnowLeopardEngine/Function/Rendering/Texture.h"

namespace SnowLeopardEngine
{
    class RenderContext;

    namespace IO
    {
        Texture* Load(const std::filesystem::path& texturePath, RenderContext& rc, bool flip = true);
        void     Release(const std::filesystem::path& texturePath, Texture& texture, RenderContext& rc);

        // Destroys every cached texture. Call while the render context is still alive.
        void ReleaseAllTextures(RenderContext& rc);

        extern std::unordered_map<size_t, Scope<Texture>> g_TextureCache;
    } // namespace IO
} // namespace SnowLeopardEngine