#pragma once

#include "SnowLeopardEngine/Function/Rendering/Texture.h"

namespace SnowLeopardEngine
{
    class RenderContext;

    namespace IO
    {
        Texture* Load(const std::filesystem::path& texturePath, RenderContext& rc, bool flip = true);

        extern std::unordered_map<size_t, Texture*> g_TextureCache;
    } // namespace IO
} // namespace SnowLeopardEngine