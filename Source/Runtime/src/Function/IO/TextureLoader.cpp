#include "SnowLeopardEngine/Function/IO/TextureLoader.h"
#include "SnowLeopardEngine/Function/Rendering/RenderContext.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

namespace SnowLeopardEngine
{
    namespace IO
    {
        Texture* Load(const std::filesystem::path& texturePath, RenderContext& rc, bool flip)
        {
            if (texturePath.empty())
            {
                return nullptr;
            }

            auto       p = std::filesystem::absolute(texturePath);
            const auto h = std::filesystem::hash_value(p);
            if (auto it = g_TextureCache.find(h); it != g_TextureCache.cend())
            {
                auto extent = it->second->GetExtent();
                assert(extent.Width > 0 && extent.Height > 0);

                return it->second;
            }

            stbi_set_flip_vertically_on_load(flip);

            auto* f = stbi__fopen(texturePath.string().c_str(), "rb");
            assert(f);

            const auto hdr = stbi_is_hdr_from_file(f);

            int32_t width, height, numChannels;
            auto*   pixels = hdr ? (void*)stbi_loadf_from_file(f, &width, &height, &numChannels, 0) :
                                   (void*)stbi_load_from_file(f, &width, &height, &numChannels, 0);
            fclose(f);
            assert(pixels);

            ImageData imageData {
                .DataType = static_cast<GLenum>(hdr ? GL_FLOAT : GL_UNSIGNED_BYTE),
                .Pixels   = pixels,
            };
            PixelFormat pixelFormat {PixelFormat::Unknown};
            switch (numChannels)
            {
                case 1:
                    imageData.Format = GL_RED;
                    pixelFormat      = PixelFormat::R8_UNorm;
                    break;
                case 3:
                    imageData.Format = GL_RGB;
                    pixelFormat      = hdr ? PixelFormat::RGB16F : PixelFormat::RGB8_UNorm;
                    break;
                case 4:
                    imageData.Format = GL_RGBA;
                    pixelFormat      = hdr ? PixelFormat::RGBA16F : PixelFormat::RGBA8_UNorm;
                    break;

                default:
                    assert(false);
            }

            uint32_t numMipLevels {1u};
            if (Math::IsPowerOf2(width) && Math::IsPowerOf2(height))
                numMipLevels = calcMipLevels(glm::max(width, height));

            auto texture = rc.CreateTexture2D(
                {static_cast<uint32_t>(width), static_cast<uint32_t>(height)}, pixelFormat, numMipLevels);
            rc.Upload(texture, 0, {width, height}, imageData)
                .SetupSampler(texture,
                              {
                                  .MinFilter     = TexelFilter::Linear,
                                  .MipmapMode    = MipmapMode::Linear,
                                  .MagFilter     = TexelFilter::Linear,
                                  .MaxAnisotropy = 16.0f,
                              });
            stbi_image_free(pixels);

            if (numMipLevels > 1)
                rc.GenerateMipmaps(texture);

            auto* newTexture  = new Texture {std::move(texture)};
            g_TextureCache[h] = newTexture;

            return newTexture;
        }

        void Release(const std::filesystem::path& texturePath, Texture& texture, RenderContext& rc)
        {
            if (texturePath.empty())
            {
                return;
            }

            auto       p = std::filesystem::absolute(texturePath);
            const auto h = std::filesystem::hash_value(p);
            if (auto it = g_TextureCache.find(h); it != g_TextureCache.cend())
            {
                rc.Destroy(texture);
                g_TextureCache.erase(h);
            }
        }

        std::unordered_map<size_t, Texture*> g_TextureCache;
    } // namespace IO
} // namespace SnowLeopardEngine