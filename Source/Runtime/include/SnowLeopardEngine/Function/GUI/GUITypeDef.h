#pragma once

#include "SnowLeopardEngine/Core/Math/Math.h"
#include "SnowLeopardEngine/Function/Rendering/Texture.h"

#include "cereal/cereal.hpp"

namespace SnowLeopardEngine::UI
{
    using RenderTarget = Texture*;

    enum ButtonTintType
    {
        Invalid = 0,
        Color,
        Texture
    };

    struct ColorTint
    {
        std::filesystem::path TargetGraphicPath;
        RenderTarget          TargetGraphic = nullptr;

        glm::vec4 Normal  = {0.9, 0.9, 0.9, 1};
        glm::vec4 Pressed = {0.7, 0.7, 0.7, 1};

        glm::vec4 Current = Normal;

        // NOLINTBEGIN
        template<class Archive>
        void serialize(Archive& archive)
        {
            archive(CEREAL_NVP(TargetGraphicPath), CEREAL_NVP(Normal), CEREAL_NVP(Pressed), CEREAL_NVP(Current));
        }
        // NOLINTEND
    };

    struct TextureTint
    {
        RenderTarget TargetGraphic = nullptr;

        std::filesystem::path NormalTexture;
        std::filesystem::path PressedTexture;
        std::filesystem::path Current;

        TextureTint() : Current(NormalTexture) {}

        // NOLINTBEGIN
        template<class Archive>
        void serialize(Archive& archive)
        {
            archive(CEREAL_NVP(NormalTexture), CEREAL_NVP(PressedTexture));
        }
        // NOLINTEND
    };
} // namespace SnowLeopardEngine::UI