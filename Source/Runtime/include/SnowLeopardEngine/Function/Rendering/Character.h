#pragma once

#include <glad/glad.h>
#include <glm/vec2.hpp>
#include <map>

namespace SnowLeopardEngine
{
    // Character structure
    struct Character
    {
        GLuint     TextureId; // ID of the glyph texture
        glm::ivec2 Size;      // Size of glyph
        glm::ivec2 Bearing;   // Offset from baseline to left/top of glyph
        GLuint     Advance;   // Offset to advance to next glyph
    };

    extern std::map<GLchar, Character> g_Characters;
} // namespace SnowLeopardEngine