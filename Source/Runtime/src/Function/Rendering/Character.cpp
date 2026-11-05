#include "SnowLeopardEngine/Function/Rendering/Character.h"
#include "SnowLeopardEngine/Engine/EngineContext.h"
#include "SnowLeopardEngine/Function/Rendering/RenderContext.h"

#include <ft2build.h>
#include FT_FREETYPE_H

namespace SnowLeopardEngine
{
    std::map<char, Character> g_Characters;

    bool LoadDefaultFontGlyphs(RenderContext& rc)
    {
        FT_Library ft;
        if (FT_Init_FreeType(&ft))
        {
            SNOW_LEOPARD_CORE_ERROR("[Font] Could not init FreeType Library");
            return false;
        }

        FT_Face face;
        if (FT_New_Face(ft, "Assets/Fonts/Jaro-Regular.ttf", 0, &face))
        {
            SNOW_LEOPARD_CORE_ERROR("[Font] Failed to load font Jaro-Regular");
            FT_Done_FreeType(ft);
            return false;
        }

        FT_Set_Pixel_Sizes(face, 0, 48);

        ReleaseFontGlyphs(rc);
        for (unsigned char c = 0; c < 128; ++c)
        {
            if (FT_Load_Char(face, c, FT_LOAD_RENDER))
            {
                SNOW_LEOPARD_CORE_ERROR("[Font] Failed to load Glyph");
                continue;
            }

            const auto& bitmap = face->glyph->bitmap;

            Character character {
                .Size    = glm::ivec2(bitmap.width, bitmap.rows),
                .Bearing = glm::ivec2(face->glyph->bitmap_left, face->glyph->bitmap_top),
                .Advance = static_cast<uint32_t>(face->glyph->advance.x),
            };

            // Rows are tightly packed; the backend keeps an unpack alignment of 1.
            if (bitmap.width > 0 && bitmap.rows > 0)
            {
                character.Glyph = rc.CreateTexture2D({bitmap.width, bitmap.rows}, PixelFormat::R8_UNorm, 1);
                rc.SetupSampler(character.Glyph,
                                {
                                    .MinFilter    = TexelFilter::Linear,
                                    .MipmapMode   = MipmapMode::None,
                                    .MagFilter    = TexelFilter::Linear,
                                    .AddressModeS = SamplerAddressMode::ClampToEdge,
                                    .AddressModeT = SamplerAddressMode::ClampToEdge,
                                })
                    .Upload(character.Glyph,
                            0,
                            {bitmap.width, bitmap.rows},
                            {
                                .Format   = ImageFormat::Red,
                                .DataType = ImageDataType::UByte,
                                .Pixels   = bitmap.buffer,
                            });
            }

            g_Characters.emplace(static_cast<char>(c), std::move(character));
        }

        FT_Done_Face(face);
        FT_Done_FreeType(ft);

        return true;
    }

    void ReleaseFontGlyphs(RenderContext& rc)
    {
        for (auto& [_, character] : g_Characters)
            rc.Destroy(character.Glyph);
        g_Characters.clear();
    }
} // namespace SnowLeopardEngine
