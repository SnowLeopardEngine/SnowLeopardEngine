#include "SnowLeopardEngine/Function/Rendering/Renderer/FontAtlas.h"
#include "SnowLeopardEngine/Engine/EngineContext.h"

#include <ft2build.h>
#include FT_FREETYPE_H

#include <algorithm>
#include <cstring>

namespace SnowLeopardEngine
{
    namespace
    {
        FT_Face Face(void* face) { return static_cast<FT_Face>(face); }
    } // namespace

    FontAtlas::FontAtlas(RHI::Device& device, const std::filesystem::path& font) : m_Device(device), m_Pixels(kSize * kSize, 0)
    {
        FT_Library library = nullptr;
        if (FT_Init_FreeType(&library) != 0)
        {
            SNOW_LEOPARD_CORE_ERROR("[FontAtlas] FreeType initialization failed");
            return;
        }
        m_Library    = library;
        FT_Face face = nullptr;
        if (FT_New_Face(library, font.generic_string().c_str(), 0, &face) != 0)
        {
            SNOW_LEOPARD_CORE_ERROR("[FontAtlas] Cannot open font {0}", font.generic_string());
            return;
        }
        m_Face = face;
        FT_Set_Pixel_Sizes(face, 0, kBaseSize);
        m_Ascender = float(face->size->metrics.ascender) / 64.0f;

        // FreeType's SDF output stores 128 on the outline; probe which side is "inside" with a solid glyph.
        if (FT_Load_Char(face, U'I', FT_LOAD_DEFAULT) == 0 && FT_Render_Glyph(face->glyph, FT_RENDER_MODE_SDF) == 0)
        {
            const auto& bitmap = face->glyph->bitmap;
            if (bitmap.width > 0 && bitmap.rows > 0)
            {
                const uint8_t center = bitmap.buffer[(bitmap.rows / 2) * bitmap.pitch + bitmap.width / 2];
                m_InsidePositive     = center >= 128;
            }
        }

        auto desc  = RHI::ColorTextureDesc({kSize, kSize}, VriFormat_R8_UNORM);
        desc.usage = VriTextureUsage_ShaderResource | VriTextureUsage_TransferDst;
        m_Texture  = CreateScope<RHI::Texture>(device, desc);

        // Printable ASCII up front; anything else when first used.
        for (char32_t c = 32; c < 127; ++c)
        {
            Get(c);
        }
        Flush();
    }

    FontAtlas::~FontAtlas()
    {
        if (m_Face != nullptr)
        {
            FT_Done_Face(Face(m_Face));
        }
        if (m_Library != nullptr)
        {
            FT_Done_FreeType(static_cast<FT_Library>(m_Library));
        }
    }

    const FontAtlas::Glyph& FontAtlas::Get(char32_t codepoint)
    {
        if (auto found = m_Glyphs.find(codepoint); found != m_Glyphs.end())
        {
            return found->second;
        }
        Glyph glyph;
        auto* face = Face(m_Face);
        if (face == nullptr || FT_Load_Char(face, codepoint, FT_LOAD_DEFAULT) != 0)
        {
            return m_Glyphs[codepoint] = glyph;
        }
        glyph.Advance = float(face->glyph->advance.x) / 64.0f;
        if (face->glyph->outline.n_points > 0 && FT_Render_Glyph(face->glyph, FT_RENDER_MODE_SDF) == 0)
        {
            const auto&    bitmap = face->glyph->bitmap;
            const uint32_t width = bitmap.width, height = bitmap.rows;
            if (m_PenX + width + 1 > kSize)
            {
                m_PenX = 1;
                m_PenY += m_RowHeight + 1;
                m_RowHeight = 0;
            }
            if (m_PenY + height + 1 > kSize)
            {
                if (!m_Full)
                {
                    SNOW_LEOPARD_CORE_WARN("[FontAtlas] Atlas full ({0} glyphs): further glyphs are not drawn", m_Glyphs.size());
                    m_Full = true;
                }
                return m_Glyphs[codepoint] = glyph;
            }
            for (uint32_t y = 0; y < height; ++y)
            {
                const uint8_t* source      = bitmap.buffer + int64_t(y) * bitmap.pitch;
                uint8_t*       destination = &m_Pixels[size_t(m_PenY + y) * kSize + m_PenX];
                for (uint32_t x = 0; x < width; ++x)
                {
                    destination[x] = m_InsidePositive ? source[x] : uint8_t(255 - source[x]);
                }
            }
            glyph.Size    = {float(width), float(height)};
            glyph.Bearing = {float(face->glyph->bitmap_left), float(face->glyph->bitmap_top)};
            glyph.UvMin   = {float(m_PenX) / kSize, float(m_PenY) / kSize};
            glyph.UvMax   = {float(m_PenX + width) / kSize, float(m_PenY + height) / kSize};
            glyph.Visible = true;
            m_PenX += width + 1;
            m_RowHeight = std::max(m_RowHeight, height);
            m_Dirty     = true;
        }
        return m_Glyphs[codepoint] = glyph;
    }

    float FontAtlas::Kerning(char32_t left, char32_t right)
    {
        auto* face = Face(m_Face);
        if (face == nullptr || !FT_HAS_KERNING(face))
        {
            return 0;
        }
        FT_Vector delta {};
        FT_Get_Kerning(face, FT_Get_Char_Index(face, left), FT_Get_Char_Index(face, right), FT_KERNING_DEFAULT, &delta);
        return float(delta.x) / 64.0f;
    }

    void FontAtlas::Flush()
    {
        if (!m_Dirty || !m_Texture)
        {
            return;
        }
        const std::span<const std::byte> level[] {std::as_bytes(std::span(m_Pixels))};
        RHI::UploadTexture(m_Device, *m_Texture, level);
        m_Dirty = false;
    }

    std::u32string FontAtlas::Decode(std::string_view utf8)
    {
        std::u32string result;
        for (size_t i = 0; i < utf8.size();)
        {
            const auto lead   = uint8_t(utf8[i]);
            uint32_t   length = lead < 0x80 ? 1 : (lead >> 5) == 0x6 ? 2 : (lead >> 4) == 0xE ? 3 : (lead >> 3) == 0x1E ? 4 : 0;
            if (length == 0 || i + length > utf8.size())
            {
                result.push_back(U'�');
                ++i;
                continue;
            }
            char32_t codepoint = length == 1 ? lead : lead & (0x7F >> length);
            bool     valid     = true;
            for (uint32_t k = 1; k < length; ++k)
            {
                const auto next = uint8_t(utf8[i + k]);
                valid           = valid && (next & 0xC0) == 0x80;
                codepoint       = (codepoint << 6) | (next & 0x3F);
            }
            result.push_back(valid ? codepoint : U'�');
            i += valid ? length : 1;
        }
        return result;
    }
} // namespace SnowLeopardEngine
