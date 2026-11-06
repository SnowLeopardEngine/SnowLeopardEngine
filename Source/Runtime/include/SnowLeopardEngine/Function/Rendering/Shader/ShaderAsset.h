#pragma once

#include "SnowLeopardEngine/Function/Rendering/Shader/ShaderProgram.h"

#include <array>
#include <filesystem>
#include <map>
#include <optional>
#include <string>
#include <variant>
#include <vector>

namespace SnowLeopardEngine
{
    // ------------------------------------------------------------------------------------------------------------
    // DzShader v3 (.dzshader): a ShaderLab-style text format with Slang program blocks, parsed by an ANTLR grammar
    // (Function/Rendering/Shader/Grammar) and compiled by Slang to SPIR-V. Ported from libvultra's .vshader.
    //
    //   Shader "Name"
    //   {
    //       Properties { tint ("Tint", Color) = (1, 1, 1, 1)   gain ("Gain", Range(0, 4)) = 1   albedo ("Albedo", 2D) = "white" {} }
    //       Variant "Name" { Defines { FOO = 1 } }                    // optional; default: one "Default" variant
    //       SubShader
    //       {
    //           Tags { "RenderType" = "Opaque" }
    //           Cull Back  ZWrite On  ZTest LEqual  Blend SrcAlpha OneMinusSrcAlpha   // render states, [property] allowed
    //           SLANGINCLUDE ... ENDSLANG                              // Slang shared by every pass
    //           Pass { Name "Forward"  Tags { "LightMode" = "Forward" }  Vertex vsMain  Fragment fsMain
    //                  SLANGPROGRAM ... ENDSLANG }
    //       }
    //   }
    //
    // Properties become fields of `ParameterBlock<DzMaterialProperties> material` (textures add `<name>Sampler`,
    // 2D textures also `<name>ScaleOffset`), so program code reads `material.tint`, `material.albedo`.
    // Surface blocks parse but compile only once the lit pipeline exists (Phase 2.4).
    // ------------------------------------------------------------------------------------------------------------

    enum class ShaderPropertyType
    {
        Float,
        Range,
        Integer,
        Boolean,
        Vector,
        Color,
        Texture2D,
        Texture2DArray,
        Texture3D,
        TextureCube,
        TextureCubeArray
    };

    struct ShaderTextureValue
    {
        std::string          Builtin; // "white", "black", "normal" or empty
        std::array<float, 4> ScaleOffset {1, 1, 0, 0};

        bool operator==(const ShaderTextureValue&) const = default;
    };

    using ShaderPropertyValue = std::variant<float, int32_t, bool, std::array<float, 4>, ShaderTextureValue>;

    struct ShaderSourceLocation
    {
        std::filesystem::path File;
        uint32_t              Line   = 1;
        uint32_t              Column = 1;

        std::string Describe() const;
    };

    struct ShaderProperty
    {
        std::string                                  Name;
        std::string                                  Label;
        ShaderPropertyType                           Type         = ShaderPropertyType::Float;
        ShaderPropertyValue                          DefaultValue = 0.0f;
        bool                                         HDR          = false;
        bool                                         Normal       = false;
        bool                                         SRGB         = false;
        bool                                         Hidden       = false;
        bool                                         NoScaleOffset = false;
        double                                       Minimum      = 0;
        double                                       Maximum      = 1;
        std::vector<std::pair<std::string, int32_t>> Enumeration;
        ShaderSourceLocation                         Location;

        bool IsTexture() const;
        // Throws if `value` does not fit this property (type, range, enumeration).
        void Validate(const ShaderPropertyValue& value) const;
    };

    struct ShaderStateValue
    {
        std::string Value;
        bool        Property = false; // `[name]`: taken from a material property
    };

    // Render-state commands as written (Cull, ZWrite, ZTest, Blend, BlendOp, ColorMask, DepthBias, FrontFace,
    // Stencil.*); resolved to VRI pipeline state by ShaderPipeline.
    struct ShaderState
    {
        std::map<std::string, std::vector<ShaderStateValue>, std::less<>> Commands;
        ShaderSourceLocation                                              Location;
    };

    struct ShaderSourceBlock
    {
        std::string          Text;
        ShaderSourceLocation Location;
    };

    struct ShaderVariant
    {
        std::string               Name;
        std::string               Constants; // Slang `export static const` declarations (link-time constants)
        std::vector<ShaderDefine> Defines;
        ShaderSourceLocation      Location;
    };

    struct ShaderPass
    {
        std::string                                       Name;
        std::string                                       LightMode = "Custom";
        std::map<std::string, std::string, std::less<>>   Tags;
        std::vector<ShaderEntry>                          Entries;
        ShaderState                                       State; // the SubShader's state, overridden by the pass
        ShaderSourceBlock                                 Source;
        std::map<std::string, ShaderProgram, std::less<>> Programs; // per variant, after Compile
        ShaderSourceLocation                              Location;

        const ShaderProgram& GetProgram(std::string_view variant = "Default") const;
    };

    struct ShaderSurface
    {
        std::string          Model;
        std::string          Entry;
        ShaderState          State;
        ShaderSourceBlock    Source;
        ShaderSourceLocation Location;
    };

    struct ShaderSubshader
    {
        std::map<std::string, std::string, std::less<>> Tags;
        ShaderState                                     State;
        std::vector<ShaderSourceBlock>                  Common;
        std::optional<ShaderSurface>                    Surface;
        std::vector<ShaderPass>                         Passes;
        ShaderSourceLocation                            Location;

        const ShaderPass* FindPass(std::string_view name) const;
    };

    // CPU-side shader. GPU objects are built from it by ShaderPipeline.
    struct ShaderAsset
    {
        std::string                  Name;
        std::filesystem::path        SourcePath;
        std::vector<ShaderProperty>  Properties;
        std::vector<ShaderVariant>   Variants;
        std::vector<ShaderSubshader> Subshaders;
        std::string                  Diagnostics; // compiler warnings

        // Parse only (syntax, properties, states, structure). Throws std::invalid_argument with file:line:column.
        static ShaderAsset Parse(const std::filesystem::path& file, std::string_view text);
        // Parse and compile every pass for every variant.
        static ShaderAsset Load(const std::filesystem::path& file, const ShaderCompileOptions& options = {});
        static ShaderAsset LoadSource(const std::filesystem::path& file,
                                      std::string_view             text,
                                      const ShaderCompileOptions&  options = {});

        const ShaderProperty& GetProperty(std::string_view name) const;
        const ShaderVariant&  GetVariant(std::string_view name) const;

        std::string GeneratedMaterialSource() const;
        std::string GeneratedPassSource(uint32_t subshader, uint32_t pass) const;
    };
} // namespace SnowLeopardEngine
