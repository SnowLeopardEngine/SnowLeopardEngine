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
        Back,
        Both
    };

    enum class BlendMode : uint8_t
    {
        Invalid = 0,
        Disable,
        Enable
    };

    enum class BlendFunc : uint16_t
    {
        Invalid = 0,
        Zero,
        One,
        SrcColor,
        OneMinusSrcColor,
        DstColor,
        OneMinusDstColor,
        SrcAlpha,
        OneMinusSrcAlpha,
        DstAlpha,
        OneMinusDstAlpha,
        ConstantColor,
        OneMinusConstantColor,
        ConstantAlpha,
        OneMinusConstantAlpha,
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

        PipelineType Type = PipelineType::Graphics;

        DepthTestMode DepthTest  = DepthTestMode::Less;
        bool          DepthWrite = true;

        StencilTestMode StencilTest = StencilTestMode::Invalid;

        CullFaceMode CullFace = CullFaceMode::Back;

        BlendMode Blend      = BlendMode::Enable;
        BlendFunc BlendFunc1 = BlendFunc::SrcAlpha;
        BlendFunc BlendFunc2 = BlendFunc::OneMinusSrcAlpha;

        PipelineFlag Flag = PipelineFlag::Invalid;
    };
} // namespace SnowLeopardEngine