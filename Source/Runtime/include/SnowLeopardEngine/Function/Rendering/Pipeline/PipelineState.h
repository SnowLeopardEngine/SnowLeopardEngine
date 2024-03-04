#pragma once

#include "SnowLeopardEngine/Core/Base/Base.h"

namespace SnowLeopardEngine
{
    enum class DepthTestMode : uint16_t
    {
        Invalid = 0,
        Disable,
        Greater,
        GreaterEqual,
        Equal,
        LessEqual,
        Less,
        NotEqual,
        Never,
        AlwaysPass
    };

    enum class StencilTestMode : uint16_t
    {
        Invalid = 0
    };

    enum class CullFaceMode : uint16_t
    {
        Invalid = 0,
        NoCull,
        Front,
        Back
    };

    enum class PipelineType : uint8_t
    {
        Invalid = 0,
        Graphics,
        Compute
    };

    enum class PipelineFlag : uint8_t
    {
        Invalid   = 0x0000,
        Shadow    = 0x0001,
        DebugDraw = 0x0010
    };

    struct PipelineState
    {
        virtual ~PipelineState() = default;

        std::string  Name;
        PipelineType Type = PipelineType::Graphics;

        std::string VertexShaderName;
        std::string FragmentShaderName;
        // TODO: Kexuan Zhang More shader names

        DepthTestMode DepthTest  = DepthTestMode::Less;
        bool          DepthWrite = true;

        StencilTestMode StencilTest = StencilTestMode::Invalid;

        CullFaceMode CullFace = CullFaceMode::Back;

        PipelineFlag Flag = PipelineFlag::Invalid;
    };
} // namespace SnowLeopardEngine