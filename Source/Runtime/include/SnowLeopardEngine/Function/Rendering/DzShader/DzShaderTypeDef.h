#pragma once

#include <cereal/archives/json.hpp>
#include <cereal/cereal.hpp>
#include <cereal/types/string.hpp>
#include <cereal/types/unordered_map.hpp>
#include <cereal/types/vector.hpp>

#include <fmt/core.h>
#include <magic_enum.hpp>

namespace SnowLeopardEngine
{
    enum class DzShaderLanguageType
    {
        Invalid = 0,
        GLSL,
        HLSL
    };

    enum class DzShaderPropertyType
    {
        Invalid = 0,
        Int,
        Float,
        Texture2D,
        Texture2DArray,
        Texture3D,
        Cubemap,
        CubemapArray,
        Color,
        Vector,
        Range
    };

    struct DzStage
    {
        std::string Name;

        std::string LanguageType;
        std::string EntryPoint = "main";
        std::string ShaderSource;

        DzStage() = default;

        // NOLINTBEGIN
        template<class Archive>
        void serialize(Archive& archive)
        {
            archive(CEREAL_NVP(Name), CEREAL_NVP(LanguageType), CEREAL_NVP(EntryPoint), CEREAL_NVP(ShaderSource));
        }
        // NOLINTEND
    };

    struct DzPipelineStates
    {
        std::string CullFaceMode    = "Back";
        std::string ZTest           = "On";
        std::string ZTestMode       = "LessEqual";
        std::string ZWrite          = "On";
        std::string StencilTest     = "Off";
        std::string StencilTestMode = "Invalid";

        // TODO: more pipeline states

        // NOLINTBEGIN
        template<class Archive>
        void serialize(Archive& archive)
        {
            archive(CEREAL_NVP(CullFaceMode),
                    CEREAL_NVP(ZTest),
                    CEREAL_NVP(ZTestMode),
                    CEREAL_NVP(ZWrite),
                    CEREAL_NVP(StencilTest),
                    CEREAL_NVP(StencilTestMode));
        }
        // NOLINTEND
    };

    struct DzTags
    {
        std::string RenderQueue = "Geometry";

        // TODO: more tags

        // NOLINTBEGIN
        template<class Archive>
        void serialize(Archive& archive)
        {
            archive(CEREAL_NVP(RenderQueue));
        }
        // NOLINTEND
    };

    struct DzPass
    {
        std::string Name;

        DzPipelineStates PipelineStates;
        DzTags           Tags;

        std::vector<DzStage> Stages;

        DzPass() = default;

        // NOLINTBEGIN
        template<class Archive>
        void serialize(Archive& archive)
        {
            archive(CEREAL_NVP(Name), CEREAL_NVP(PipelineStates), CEREAL_NVP(Tags), CEREAL_NVP(Stages));
        }
        // NOLINTEND
    };

    struct DzShaderProperty
    {
        std::string Name;
        std::string Type;
        std::string Value;

        DzShaderProperty() = default;
        DzShaderProperty(std::string_view name, std::string_view type, const std::string& value) :
            Name(name), Type(type), Value(value)
        {}

        // NOLINTBEGIN
        template<class Archive>
        void serialize(Archive& archive)
        {
            archive(CEREAL_NVP(Name), CEREAL_NVP(Type), CEREAL_NVP(Value));
        }
        // NOLINTEND
    };

    struct DzShader
    {
        std::string Name;

        std::vector<DzShaderProperty> Properties;

        DzPipelineStates         PipelineStates;
        DzTags                   Tags;
        std::vector<std::string> Keywords; // aka. Macros

        std::vector<DzPass> Passes;

        DzShader() = default;

        // NOLINTBEGIN
        template<class Archive>
        void serialize(Archive& archive)
        {
            archive(CEREAL_NVP(Name),
                    CEREAL_NVP(Properties),
                    CEREAL_NVP(PipelineStates),
                    CEREAL_NVP(Tags),
                    CEREAL_NVP(Keywords),
                    CEREAL_NVP(Passes));
        }
        // NOLINTEND
    };
} // namespace SnowLeopardEngine