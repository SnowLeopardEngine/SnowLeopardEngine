#pragma once

#include "SnowLeopardEngine/Function/Rendering/RenderTypeDef.h"

namespace SnowLeopardEngine
{
    struct DepthStencilState
    {
        bool      DepthTest {false};
        bool      DepthWrite {true};
        CompareOp DepthCompareOp {CompareOp::Less};

        // clang-format off
        auto operator<=> (const DepthStencilState&) const = default;
        // clang-format on
    };

    enum class BlendOp : uint8_t
    {
        Add = 0,
        Subtract,
        ReverseSubtract,
        Min,
        Max
    };
    enum class BlendFactor : uint8_t
    {
        Zero = 0,
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
        SrcAlphaSaturate,
        Src1Color,
        OneMinusSrc1Color,
        Src1Alpha,
        OneMinusSrc1Alpha
    };

    // src = incoming values
    // dest = values that are already in a framebuffer
    struct BlendState
    {
        bool Enabled {false};

        BlendFactor SrcColor {BlendFactor::One};
        BlendFactor DestColor {BlendFactor::Zero};
        BlendOp     ColorOp {BlendOp::Add};

        BlendFactor SrcAlpha {BlendFactor::One};
        BlendFactor DestAlpha {BlendFactor::Zero};
        BlendOp     AlphaOp {BlendOp::Add};

        // clang-format off
        auto operator<=> (const BlendState&) const = default;
        // clang-format on
    };
    constexpr auto kMaxNumBlendStates = 4;

    enum class PolygonMode : uint8_t
    {
        Point = 0,
        Line,
        Fill
    };
    enum class CullMode : uint8_t
    {
        None = 0,
        Back,
        Front
    };
    struct PolygonOffset
    {
        float Factor {0.0f}, Units {0.0f};

        // clang-format off
        auto operator<=> (const PolygonOffset&) const = default;
        // clang-format on
    };

    struct RasterizerState
    {
        PolygonMode                  PolygonMode {PolygonMode::Fill};
        CullMode                     CullMode {CullMode::Back};
        std::optional<PolygonOffset> PolygonOffset;
        bool                         DepthClampEnable {false};
        bool                         ScissorTest {false};

        // clang-format off
        auto operator<=> (const RasterizerState&) const = default;
        // clang-format on
    };
} // namespace SnowLeopardEngine