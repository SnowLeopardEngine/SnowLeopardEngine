#include "SnowLeopardEngine/Function/Asset/Loaders/TextureLoader.h"
#include "SnowLeopardEngine/Core/Log/LogSystem.h"
#include "SnowLeopardEngine/Engine/EngineContext.h"
#include "SnowLeopardEngine/Function/Rendering/RHI/Texture.h"
#include <vector>

#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>

namespace SnowLeopardEngine
{
    bool TextureLoader::LoadTexture2D(const std::filesystem::path& path, bool flip, TextureLoadingOutput& output)
    {
        std::string pathString = path.generic_string();

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
        TextureLoadingOutput output;
        if (!LoadTexture2D(path, flip, output))
        {
            return nullptr;
        }

        return Texture2D::Create(output.GetDesc(), &output.OutBuffer);
    }

    Ref<Texture3D> TextureLoader::LoadTexture3D(std::vector<std::filesystem::path> facePaths, bool flip)
    {
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

        return Texture3D::Create(outputs[0].GetDesc(), dataList);
    }
} // namespace SnowLeopardEngine