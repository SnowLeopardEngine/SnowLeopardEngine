#pragma once

#include "SnowLeopardEngine/Core/Base/Base.h"

#include <vri/vri.h>

#include <array>
#include <cstdint>
#include <filesystem>
#include <optional>
#include <span>
#include <string>
#include <string_view>
#include <vector>

namespace SnowLeopardEngine
{
    struct ShaderEntry
    {
        std::string        Name;
        VriShaderStageBits Stage = VriShaderStage_Vertex;
    };

    struct CompiledShader
    {
        ShaderEntry             Entry;
        std::vector<uint32_t>   Words; // SPIR-V
        uint32_t                ColorOutputs = 0;
        std::array<uint32_t, 3> ThreadGroup {};
    };

    enum class ShaderParameterKind
    {
        None,
        Scalar,
        Vector,
        Matrix,
        Struct,
        Array,
        ConstantBuffer,
        ParameterBlock,
        Resource,
        Sampler
    };

    enum class ShaderScalarType
    {
        None,
        Float,
        Integer,
        Unsigned,
        Boolean,
        Other
    };

    enum class ShaderOffsetKind
    {
        Uniform,
        Descriptor,
        RegisterSpace,
        PushConstant
    };

    struct ShaderParameterOffset
    {
        ShaderOffsetKind Kind;
        uint64_t         Offset = 0;
        uint64_t         Space  = 0;
    };

    // Reflected parameter layout (Slang), normalized to VRI. Constant buffers / parameter blocks keep their
    // "$container" and "$element" children; arrays and structured buffers an "$element" child.
    struct ShaderParameter
    {
        std::string                        Name;
        std::string                        TypeName;
        ShaderParameterKind                Kind         = ShaderParameterKind::None;
        ShaderScalarType                   Scalar       = ShaderScalarType::None;
        uint64_t                           Size         = 0;
        uint64_t                           Stride       = 0;
        uint64_t                           Count        = 0;
        uint64_t                           MatrixStride = 0;
        uint32_t                           Rows         = 0;
        uint32_t                           Columns      = 0;
        bool                               ColumnMajor  = true;
        std::optional<VriDescriptorType>   DescriptorType;
        VriTextureViewType                 ViewType = VriTextureViewType_2D;
        std::vector<ShaderParameterOffset> Offsets;
        std::vector<ShaderParameter>       Fields;

        uint64_t               Offset(ShaderOffsetKind kind) const;
        uint64_t               Space(ShaderOffsetKind kind) const;
        const ShaderParameter* Field(std::string_view name) const;
    };

    struct ShaderDefine
    {
        std::string Name;
        std::string Value;
    };

    struct ShaderCompileOptions
    {
        std::vector<ShaderEntry>           Entries; // empty: every annotated entry point
        std::vector<std::filesystem::path> IncludeDirectories;
        std::vector<ShaderDefine>          Defines;
        std::vector<ShaderDefine>          LinkSources; // extra Slang modules: name + source text
        std::string                        Profile = "spirv_1_5";
    };

    // One descriptor (or push-constant block) a program uses, with its VRI set / binding.
    struct ShaderResourceBinding
    {
        std::string        Name;
        VriDescriptorType  Type         = VriDescriptorType_Texture;
        uint32_t           Set          = 0;
        uint32_t           Binding      = 0;
        uint32_t           Count        = 1;
        uint64_t           UniformSize  = 0; // constant buffers / push constants
        VriTextureViewType ViewType     = VriTextureViewType_2D;
        bool               PushConstant = false;
    };

    struct ShaderProgram
    {
        std::vector<CompiledShader> Shaders;
        std::string                 Diagnostics;
        std::string                 Profile = "spirv_1_5";
        ShaderParameter             Parameters; // "$globals"

        // VRI shader descriptors for `entries` (pointing into this program; keep it alive while creating a pipeline).
        std::vector<VriShaderDesc>         Descriptors(std::span<const ShaderEntry> entries) const;
        std::vector<ShaderResourceBinding> ResourceBindings() const;
        const CompiledShader*              Find(VriShaderStageBits stage) const;
    };

    // Slang -> SPIR-V with reflection, ported from libvultra's ShaderCompiler (no disk / IR cache yet).
    // One owner; Slang's global session is created on first use and reused.
    class ShaderCompiler
    {
    public:
        ShaderCompiler();
        ~ShaderCompiler();

        ShaderCompiler(const ShaderCompiler&)            = delete;
        ShaderCompiler& operator=(const ShaderCompiler&) = delete;

        ShaderProgram Compile(const std::filesystem::path& file, const ShaderCompileOptions& options = {});
        // `file` names the source for diagnostics and relative includes; it does not have to exist.
        ShaderProgram CompileSource(const std::filesystem::path& file,
                                    std::string_view             text,
                                    const ShaderCompileOptions&  options = {});

    private:
        struct Impl;
        Scope<Impl> m_Impl;
    };
} // namespace SnowLeopardEngine
