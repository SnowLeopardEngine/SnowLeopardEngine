#include "SnowLeopardEngine/Function/IO/TextureLoader.h"
#include "SnowLeopardEngine/Engine/EngineContext.h"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include <unordered_map>

namespace SnowLeopardEngine
{
    namespace IO
    {
        namespace
        {
            std::unordered_map<std::string, Scope<RHI::Texture>> g_TextureCache;
        }

        RHI::Texture* Load(const std::filesystem::path& texturePath, bool flip)
        {
            const auto key = texturePath.generic_string() + (flip ? "|flip" : "");
            if (auto it = g_TextureCache.find(key); it != g_TextureCache.end())
            {
                return it->second.get();
            }

            stbi_set_flip_vertically_on_load(flip);
            int  width = 0, height = 0, channels = 0;
            auto pixels = stbi_load(texturePath.generic_string().c_str(), &width, &height, &channels, STBI_rgb_alpha);
            if (pixels == nullptr)
            {
                SNOW_LEOPARD_CORE_ERROR("[TextureLoader] Failed to load {0}", texturePath.generic_string());
                return nullptr;
            }

            auto& device = g_EngineContext->RenderSys->GetDevice();
            auto  desc   = RHI::ColorTextureDesc({uint32_t(width), uint32_t(height)}, VriFormat_RGBA8_UNORM);
            desc.usage   = VriTextureUsage_ShaderResource | VriTextureUsage_TransferDst;
            auto texture = CreateScope<RHI::Texture>(device, desc);
            RHI::UploadTexture2D(
                device,
                *texture,
                std::span(reinterpret_cast<const std::byte*>(pixels), size_t(width) * size_t(height) * 4));
            stbi_image_free(pixels);

            auto* result         = texture.get();
            g_TextureCache[key] = std::move(texture);
            return result;
        }

        void ReleaseAllTextures() { g_TextureCache.clear(); }
    } // namespace IO
} // namespace SnowLeopardEngine
