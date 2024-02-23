#pragma once

#include "SnowLeopardEngine/Core/Base/Base.h"

namespace SnowLeopardEngine
{
    enum class DepthTestMode : uint16_t
    {
        None = 0,
        Greater,
        GreaterEqual,
        Equal,
        LessEqual,
        Less,
        NotEqual,
        Never,
        Always
    };

    enum class StencilTestMode : uint16_t
    {
        None = 0
    };

    enum class CullFaceMode : uint16_t
    {
        None = 0,
        Front,
        Back
    };

    enum class PipelineType : uint8_t
    {
        Graphics = 0,
        Compute
    };

    enum class PipelineFlag : uint8_t
    {
        None      = 0x0000,
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

        StencilTestMode StencilTest = StencilTestMode::None;

        CullFaceMode CullFace = CullFaceMode::Back;

        PipelineFlag Flag = PipelineFlag::None;
    };
} // namespace SnowLeopardEngine