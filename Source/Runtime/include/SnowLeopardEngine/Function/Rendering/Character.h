#pragma once

#include "SnowLeopardEngine/Function/Rendering/RenderHandle.h"

#include <glm/vec2.hpp>
#include <map>

namespace SnowLeopardEngine
{
    // Character structure
    struct Character
    {
        RenderHandle TextureId; // ID of the glyph texture
        glm::ivec2   Size;      // Size of glyph
        glm::ivec2   Bearing;   // Offset from baseline to left/top of glyph
        uint32_t     Advance;   // Offset to advance to next glyph
    };

    extern std::map<char, Character> g_Characters;
} // namespace SnowLeopardEngine