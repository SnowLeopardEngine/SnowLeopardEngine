#pragma once

#include "SnowLeopardEngine/Core/Math/Math.h"
#include "SnowLeopardEngine/Core/UUID/CoreUUID.h"
#include "cereal/cereal.hpp"
#include <string>

namespace SnowLeopardEngine::UI
{
    enum ButtonTintType
    {
        Invalid = 0,
        Color,
        Texture
    };

    struct ColorTint
    {
        CoreUUID TargetGraphicUUID;

        glm::vec4 Normal  = {0.9, 0.9, 0.9, 1};
        glm::vec4 Pressed = {0.7, 0.7, 0.7, 1};

        glm::vec4 Current = Normal;

        // NOLINTBEGIN
        template<class Archive>
        void serialize(Archive& archive)
        {
            archive(CEREAL_NVP(Normal), CEREAL_NVP(Pressed), CEREAL_NVP(Current));
        }
        // NOLINTEND
    };

    struct TextureTint
    {
        CoreUUID TargetGraphicUUID;

        std::string NormalTexture;
        std::string PressedTexture;
        std::string Current ;

        TextureTint() : Current(NormalTexture) {}
        
        template<class Archive>
        void serialize(Archive& archive)
        {
            archive(CEREAL_NVP(NormalTexture), CEREAL_NVP(PressedTexture), CEREAL_NVP(Current));
        }

    };
} // namespace SnowLeopardEngine::UI