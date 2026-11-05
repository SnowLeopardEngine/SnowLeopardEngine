#pragma once

#include "SnowLeopardEngine/Function/Rendering/Texture.h"

#include <glm/vec2.hpp>
#include <map>

namespace SnowLeopardEngine
{
    class RenderContext;

    // Character structure
    struct Character
    {
        Texture    Glyph;   // Glyph coverage (R8). Empty for glyphs without pixels, e.g. space.
        glm::ivec2 Size;    // Size of glyph
        glm::ivec2 Bearing; // Offset from baseline to left/top of glyph
        uint32_t   Advance; // Offset to advance to next glyph, in 1/64 pixels
    };

    extern std::map<char, Character> g_Characters;

    // Rasterizes ASCII glyphs of the default UI font into g_Characters.
    bool LoadDefaultFontGlyphs(RenderContext& rc);
    // Destroys the glyph textures; call while the render context is still alive.
    void ReleaseFontGlyphs(RenderContext& rc);
} // namespace SnowLeopardEngine
