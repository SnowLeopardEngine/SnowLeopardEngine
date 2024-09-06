#pragma once

#include "SnowLeopardEngine/Function/Rendering/RenderTypeDef.h"

#include <glad/glad.h>

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

    enum class BlendOp : GLenum
    {
        Add             = GL_FUNC_ADD,
        Subtract        = GL_FUNC_SUBTRACT,
        ReverseSubtract = GL_FUNC_REVERSE_SUBTRACT,
        Min             = GL_MIN,
        Max             = GL_MAX
    };
    enum class BlendFactor : GLenum
    {
        Zero                  = GL_ZERO,
        One                   = GL_ONE,
        SrcColor              = GL_SRC_COLOR,
        OneMinusSrcColor      = GL_ONE_MINUS_SRC_COLOR,
        DstColor              = GL_DST_COLOR,
        OneMinusDstColor      = GL_ONE_MINUS_DST_COLOR,
        SrcAlpha              = GL_SRC_ALPHA,
        OneMinusSrcAlpha      = GL_ONE_MINUS_SRC_ALPHA,
        DstAlpha              = GL_DST_ALPHA,
        OneMinusDstAlpha      = GL_ONE_MINUS_DST_ALPHA,
        ConstantColor         = GL_CONSTANT_COLOR,
        OneMinusConstantColor = GL_ONE_MINUS_CONSTANT_COLOR,
        ConstantAlpha         = GL_CONSTANT_ALPHA,
        OneMinusConstantAlpha = GL_ONE_MINUS_CONSTANT_ALPHA,
        SrcAlphaSaturate      = GL_SRC_ALPHA_SATURATE,
        Src1Color             = GL_SRC1_COLOR,
        OneMinusSrc1Color     = GL_ONE_MINUS_SRC1_COLOR,
        Src1Alpha             = GL_SRC1_ALPHA,
        OneMinusSrc1Alpha     = GL_ONE_MINUS_SRC1_ALPHA
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

    enum class PolygonMode : GLenum
    {
        Point = GL_POINT,
        Line  = GL_LINE,
        Fill  = GL_FILL
    };
    enum class CullMode : GLenum
    {
        None  = GL_NONE,
        Back  = GL_BACK,
        Front = GL_FRONT
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