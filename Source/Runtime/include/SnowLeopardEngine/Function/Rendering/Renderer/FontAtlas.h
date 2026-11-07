#pragma once

#include "SnowLeopardEngine/Core/Base/Base.h"
#include "SnowLeopardEngine/Core/Math/Math.h"
#include "SnowLeopardEngine/Function/Rendering/RHI/Resources.h"

#include <filesystem>
#include <string_view>
#include <unordered_map>
#include <vector>

namespace SnowLeopardEngine
{
    // Signed-distance-field glyph atlas of one font file (FreeType's SDF renderer). Glyphs are rasterized once at
    // kBaseSize pixels and scale to any size with smooth edges; they are added on demand (any Unicode code point
    // the font has) and the atlas texture is re-uploaded once per frame that added some.
    class FontAtlas
    {
    public:
        static constexpr uint32_t kBaseSize = 48;   // em size the SDF is rasterized at, in pixels
        static constexpr uint32_t kSpread   = 8;    // SDF spread in atlas pixels
        static constexpr uint32_t kSize     = 1024; // atlas width and height

        struct Glyph
        {
            glm::vec2 Size {0};    // quad size at kBaseSize, pixels (includes the SDF spread)
            glm::vec2 Bearing {0}; // quad offset from the pen: x right, y up from the baseline
            float     Advance = 0;
            glm::vec2 UvMin {0}, UvMax {0};
            bool      Visible = false; // has pixels (a space has none)
        };

        FontAtlas(RHI::Device& device, const std::filesystem::path& font);
        ~FontAtlas();

        FontAtlas(const FontAtlas&)            = delete;
        FontAtlas& operator=(const FontAtlas&) = delete;

        bool Valid() const { return m_Face != nullptr; }

        const Glyph& Get(char32_t codepoint);
        float        Kerning(char32_t left, char32_t right); // at kBaseSize
        float        Ascender() const { return m_Ascender; }

        // Uploads glyphs added since the last call (before the frame is recorded).
        void          Flush();
        RHI::Texture* GetTexture() const { return m_Texture.get(); }

        // UTF-8 -> code points; invalid bytes become U+FFFD.
        static std::u32string Decode(std::string_view utf8);

    private:
        RHI::Device&                         m_Device;
        void*                                m_Library = nullptr; // FT_Library
        void*                                m_Face    = nullptr; // FT_Face
        float                                m_Ascender = 0;
        bool                                 m_InsidePositive = true; // FreeType's SDF sign convention, probed once
        std::unordered_map<char32_t, Glyph>  m_Glyphs;
        std::vector<uint8_t>                 m_Pixels;
        uint32_t                             m_PenX = 1, m_PenY = 1, m_RowHeight = 0; // shelf packer
        bool                                 m_Dirty = true;
        bool                                 m_Full  = false;
        Scope<RHI::Texture>                  m_Texture;
    };
} // namespace SnowLeopardEngine
