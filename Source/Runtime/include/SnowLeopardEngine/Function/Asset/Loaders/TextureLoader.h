#pragma once

#include "SnowLeopardEngine/Core/Containers/Buffer.h"
#include "SnowLeopardEngine/Function/Rendering/RHI/Texture.h"
#include "SnowLeopardEngine/Function/Rendering/RenderTypeDef.h"

namespace SnowLeopardEngine
{
    struct TextureLoadingOutput
    {
        Buffer           OutBuffer;
        PixelColorFormat OutFormat;
        uint32_t         OutWidth;
        uint32_t         OutHeight;

        TextureDesc GetDesc() { return {OutWidth, OutHeight, OutFormat}; }
    };

    class TextureLoader
    {
    public:
        static bool LoadTexture2D(const std::filesystem::path& path, bool flip, TextureLoadingOutput& output);
        static bool LoadTexture3D(std::vector<std::filesystem::path> facePaths,
                                  bool                               flip,
                                  std::vector<TextureLoadingOutput>& outputs);

        static Ref<Texture2D> LoadTexture2D(const std::filesystem::path& path, bool flip);
        static Ref<Texture3D> LoadTexture3D(std::vector<std::filesystem::path> facePaths, bool flip);
    };
} // namespace SnowLeopardEngine