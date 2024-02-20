#pragma once

#include "SnowLeopardEngine/Core/Base/Base.h"

namespace SnowLeopardEngine
{
    enum class DepthTestMode
    {
        None = 0,
        Large,
        LargeEqual,
        Equal,
        LessEqual,
        Less,
        NotEqual,
        Never,
        Always
    };

    enum class StencilTestMode
    {
        None = 0
    };

    enum class CullFaceMode
    {
        None = 0,
        Front,
        Back
    };

    enum class PipelineType
    {
        Rasterisation,
        Computation
    };

    enum class PipelineFlag
    {
        None      = 0x0000,
        Shadow    = 0x0001,
        DebugDraw = 0x0010
    };

    struct PipelineState
    {
        virtual ~PipelineState() = default;

        std::string  Name;
        PipelineType Type = PipelineType::Rasterisation;

        std::string VertexShaderName;
        std::string FragmentShaderName;
        // TODO: Kexuan Zhang More shader names

        DepthTestMode DepthTest  = DepthTestMode::Always;
        bool          DepthWrite = true;

        StencilTestMode StencilTest = StencilTestMode::None;

        CullFaceMode CullFace = CullFaceMode::Back;

        PipelineFlag Flag = PipelineFlag::None;
    };
} // namespace SnowLeopardEngine