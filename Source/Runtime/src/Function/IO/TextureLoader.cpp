#include "SnowLeopardEngine/Function/IO/TextureLoader.h"
#include "SnowLeopardEngine/Engine/EngineContext.h"
#include "SnowLeopardEngine/Function/Asset/AssetManager.h"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include <algorithm>
#include <bit>
#include <unordered_map>
#include <vector>

namespace SnowLeopardEngine
{
    namespace IO
    {
        namespace
        {
            std::unordered_map<std::string, Scope<RHI::Texture>> g_TextureCache;

            // 2 x 2 box filter of an RGBA8 level (odd edges clamp).
            std::vector<std::byte> Downsample(const std::vector<std::byte>& source, uint32_t width, uint32_t height)
            {
                const uint32_t         w = std::max(width / 2, 1u), h = std::max(height / 2, 1u);
                std::vector<std::byte> result(size_t(w) * h * 4);
                for (uint32_t y = 0; y < h; ++y)
                {
                    for (uint32_t x = 0; x < w; ++x)
                    {
                        const uint32_t x0 = std::min(x * 2, width - 1), x1 = std::min(x * 2 + 1, width - 1);
                        const uint32_t y0 = std::min(y * 2, height - 1), y1 = std::min(y * 2 + 1, height - 1);
                        for (uint32_t c = 0; c < 4; ++c)
                        {
                            const auto at = [&](uint32_t sx, uint32_t sy) {
                                return uint32_t(source[(size_t(sy) * width + sx) * 4 + c]);
                            };
                            result[(size_t(y) * w + x) * 4 + c] =
                                std::byte((at(x0, y0) + at(x1, y0) + at(x0, y1) + at(x1, y1) + 2) / 4);
                        }
                    }
                }
                return result;
            }
        } // namespace

        RHI::Texture* LoadTexture(const std::filesystem::path& texturePath)
        {
            if (const auto id = AssetManager::FindBySource(texturePath))
            {
                if (auto* texture = AssetManager::LoadTexture(*id))
                {
                    return texture;
                }
            }

            const auto key = texturePath.generic_string();
            if (auto it = g_TextureCache.find(key); it != g_TextureCache.end())
            {
                return it->second.get();
            }
            AssetManager::ReportFallback(texturePath);

            stbi_set_flip_vertically_on_load(false);
            int  width = 0, height = 0, channels = 0;
            auto pixels = stbi_load(texturePath.generic_string().c_str(), &width, &height, &channels, STBI_rgb_alpha);
            if (pixels == nullptr)
            {
                SNOW_LEOPARD_CORE_ERROR("[TextureLoader] Failed to load {0}", texturePath.generic_string());
                return nullptr;
            }

            // Full mip chain, built on the CPU (box filter): minified textures do not alias.
            std::vector<std::vector<std::byte>> levels;
            levels.emplace_back(reinterpret_cast<const std::byte*>(pixels),
                                reinterpret_cast<const std::byte*>(pixels) + size_t(width) * size_t(height) * 4);
            stbi_image_free(pixels);
            uint32_t w = uint32_t(width), h = uint32_t(height);
            while (w > 1 || h > 1)
            {
                levels.push_back(Downsample(levels.back(), w, h));
                w = std::max(w / 2, 1u);
                h = std::max(h / 2, 1u);
            }

            auto& device = g_EngineContext->RenderSys->GetDevice();
            auto  desc   = RHI::ColorTextureDesc({uint32_t(width), uint32_t(height)}, VriFormat_RGBA8_UNORM);
            desc.usage   = VriTextureUsage_ShaderResource | VriTextureUsage_TransferDst;
            desc.mipNum  = uint32_t(levels.size());
            auto texture = CreateScope<RHI::Texture>(device, desc);
            std::vector<std::span<const std::byte>> spans(levels.begin(), levels.end());
            RHI::UploadTexture(device, *texture, spans);

            auto* result        = texture.get();
            g_TextureCache[key] = std::move(texture);
            return result;
        }

        RHI::Texture* LoadHDR(const std::filesystem::path& texturePath)
        {
            if (const auto id = AssetManager::FindBySource(texturePath))
            {
                if (auto* texture = AssetManager::LoadTexture(*id))
                {
                    return texture;
                }
            }
            AssetManager::ReportFallback(texturePath);
            const auto key = texturePath.generic_string() + "|hdr";
            if (auto it = g_TextureCache.find(key); it != g_TextureCache.end())
            {
                return it->second.get();
            }

            // Equirectangular maps are stored top row first, as sampled by EnvironmentUv: no flip.
            stbi_set_flip_vertically_on_load(false);
            int  width = 0, height = 0, channels = 0;
            auto pixels = stbi_loadf(texturePath.generic_string().c_str(), &width, &height, &channels, STBI_rgb_alpha);
            if (pixels == nullptr)
            {
                SNOW_LEOPARD_CORE_ERROR("[TextureLoader] Failed to load HDR {0}", texturePath.generic_string());
                return nullptr;
            }

            auto& device = g_EngineContext->RenderSys->GetDevice();
            auto  desc   = RHI::ColorTextureDesc({uint32_t(width), uint32_t(height)}, VriFormat_RGBA32_SFLOAT);
            desc.usage   = VriTextureUsage_ShaderResource | VriTextureUsage_TransferDst;
            auto texture = CreateScope<RHI::Texture>(device, desc);
            const std::span<const std::byte> level[] {
                std::span(reinterpret_cast<const std::byte*>(pixels), size_t(width) * size_t(height) * 16)};
            RHI::UploadTexture(device, *texture, level);
            stbi_image_free(pixels);

            auto* result        = texture.get();
            g_TextureCache[key] = std::move(texture);
            return result;
        }

        void ReleaseAllTextures() { g_TextureCache.clear(); }
    } // namespace IO
} // namespace SnowLeopardEngine
