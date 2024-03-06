#include "SnowLeopardEngine/Function/Asset/Loaders/TextureLoader.h"
#include "SnowLeopardEngine/Engine/EngineContext.h"
#include "SnowLeopardEngine/Function/Rendering/RHI/Texture.h"
#include "SnowLeopardEngine/Platform/Platform.h"

#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>

#if SNOW_LEOPARD_PLATFORM_DARWIN
namespace std
{
    template<>
    struct hash<filesystem::path>
    {
        size_t operator()(const filesystem::path& p) const { return hash<filesystem::path::string_type>()(p.native()); }
    };
} // namespace std
#endif

namespace SnowLeopardEngine
{
    std::unordered_map<std::string, Ref<Texture2D>> TextureLoader::s_Texture2DCache;
    std::unordered_map<size_t, Ref<Texture3D>>      TextureLoader::s_Texture3DCache;

    bool TextureLoader::LoadTexture2D(const std::filesystem::path& path, bool flip, TextureLoadingOutput& output)
    {
        std::string pathString = path.generic_string();

        if (pathString.empty())
        {
            SNOW_LEOPARD_CORE_ERROR("[TextureLoader] Empty Path!");
            return false;
        }

        stbi_set_flip_vertically_on_load(flip);

        int width, height, channels;
        if (stbi_is_hdr(pathString.c_str()))
        {
            output.OutBuffer.Data =
                reinterpret_cast<std::byte*>(stbi_loadf(pathString.c_str(), &width, &height, &channels, 4));
            output.OutBuffer.Size = width * height * 4 * sizeof(float);
            output.OutFormat      = PixelColorFormat::RGBA32;
        }
        else
        {
            output.OutBuffer.Data = stbi_load(pathString.c_str(), &width, &height, &channels, 4);
            output.OutBuffer.Size = width * height * 4;
            output.OutFormat      = PixelColorFormat::RGBA8;
        }

        if (!output.OutBuffer.Data)
        {
            SNOW_LEOPARD_CORE_ERROR("[TextureLoader] Failed to load texture from: {0}", path.generic_string());
            return false;
        }

        output.OutWidth  = width;
        output.OutHeight = height;

        SNOW_LEOPARD_CORE_INFO("[TextureLoader] Loaded texture from: {0}", path.generic_string());
        return true;
    }

    bool TextureLoader::LoadTexture3D(std::vector<std::filesystem::path> facePaths,
                                      bool                               flip,
                                      std::vector<TextureLoadingOutput>& outputs)
    {
        for (const auto& facePath : facePaths)
        {
            TextureLoadingOutput output;
            if (!LoadTexture2D(facePath, flip, output))
            {
                return false;
            }

            outputs.emplace_back(output);
        }

        return true;
    }

    Ref<Texture2D> TextureLoader::LoadTexture2D(const std::filesystem::path& path, bool flip)
    {
        if (s_Texture2DCache.count(path.generic_string()) > 0)
        {
            return s_Texture2DCache[path.generic_string()];
        }

        TextureLoadingOutput output;
        if (!LoadTexture2D(path, flip, output))
        {
            return nullptr;
        }

        auto texture = Texture2D::Create(output.GetDesc(), &output.OutBuffer);

        s_Texture2DCache[path.generic_string()] = texture;

        return texture;
    }

    Ref<Texture3D> TextureLoader::LoadTexture3D(std::vector<std::filesystem::path> facePaths, bool flip)
    {
        size_t hashValue = 0;
        for (const auto& path : facePaths)
        {
            hashValue ^= std::hash<std::filesystem::path> {}(path) + 0x9e3779b9 + (hashValue << 6) + (hashValue >> 2);
        }

        if (s_Texture3DCache.count(hashValue) > 0)
        {
            return s_Texture3DCache[hashValue];
        }

        std::vector<TextureLoadingOutput> outputs;
        if (!LoadTexture3D(facePaths, flip, outputs))
        {
            return nullptr;
        }

        if (outputs.size() != 6)
        {
            return nullptr;
        }

        std::vector<Buffer*> dataList;
        dataList.reserve(outputs.size());
        for (auto& output : outputs)
        {
            dataList.emplace_back(&output.OutBuffer);
        }

        auto texture = Texture3D::Create(outputs[0].GetDesc(), dataList);

        s_Texture3DCache[hashValue] = texture;

        return texture;
    }
} // namespace SnowLeopardEngine