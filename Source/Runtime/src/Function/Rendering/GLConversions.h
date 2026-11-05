#pragma once

// Private to the OpenGL backend: maps engine rendering enums to OpenGL enums.

#include "SnowLeopardEngine/Function/Rendering/Pipeline/PipelineState.h"
#include "SnowLeopardEngine/Function/Rendering/RenderContext.h"
#include "SnowLeopardEngine/Function/Rendering/Texture.h"

#include <glad/glad.h>

#include <cassert>

namespace SnowLeopardEngine::GL
{
    inline GLenum ToGL(CompareOp op)
    {
        switch (op)
        {
            using enum CompareOp;
            case Never:
                return GL_NEVER;
            case Less:
                return GL_LESS;
            case Equal:
                return GL_EQUAL;
            case LessOrEqual:
                return GL_LEQUAL;
            case Greater:
                return GL_GREATER;
            case NotEqual:
                return GL_NOTEQUAL;
            case GreaterOrEqual:
                return GL_GEQUAL;
            case Always:
                return GL_ALWAYS;
        }
        assert(false);
        return GL_NONE;
    }

    inline GLenum ToGL(TexelFilter filter) { return filter == TexelFilter::Nearest ? GL_NEAREST : GL_LINEAR; }

    inline GLenum ToGL(SamplerAddressMode mode)
    {
        switch (mode)
        {
            using enum SamplerAddressMode;
            case Repeat:
                return GL_REPEAT;
            case MirroredRepeat:
                return GL_MIRRORED_REPEAT;
            case ClampToEdge:
                return GL_CLAMP_TO_EDGE;
            case ClampToBorder:
                return GL_CLAMP_TO_BORDER;
            case MirrorClampToEdge:
                return GL_MIRROR_CLAMP_TO_EDGE;
        }
        assert(false);
        return GL_NONE;
    }

    // Sized internal format.
    inline GLenum ToGL(PixelFormat format)
    {
        switch (format)
        {
            using enum PixelFormat;
            case Unknown:
                return GL_NONE;
            case R8_UNorm:
                return GL_R8;
            case R32I:
                return GL_R32I;
            case RGB8_UNorm:
                return GL_RGB8;
            case RGBA8_UNorm:
                return GL_RGBA8;
            case RGB8_SNorm:
                return GL_RGB8_SNORM;
            case RGBA8_SNorm:
                return GL_RGBA8_SNORM;
            case R16F:
                return GL_R16F;
            case RG16F:
                return GL_RG16F;
            case RGB16F:
                return GL_RGB16F;
            case RGBA16F:
                return GL_RGBA16F;
            case RGB32F:
                return GL_RGB32F;
            case RGBA32F:
                return GL_RGBA32F;
            case RGBA32UI:
                return GL_RGBA32UI;
            case Depth16:
                return GL_DEPTH_COMPONENT16;
            case Depth24:
                return GL_DEPTH_COMPONENT24;
            case Depth32F:
                return GL_DEPTH_COMPONENT32F;
        }
        assert(false);
        return GL_NONE;
    }

    // Texture target.
    inline GLenum ToGL(TextureType type)
    {
        switch (type)
        {
            using enum TextureType;
            case Unknown:
                return GL_NONE;
            case Texture1D:
                return GL_TEXTURE_1D;
            case Texture1DArray:
                return GL_TEXTURE_1D_ARRAY;
            case Texture2D:
                return GL_TEXTURE_2D;
            case Texture2DArray:
                return GL_TEXTURE_2D_ARRAY;
            case Texture3D:
                return GL_TEXTURE_3D;
            case CubeMap:
                return GL_TEXTURE_CUBE_MAP;
            case CubeMapArray:
                return GL_TEXTURE_CUBE_MAP_ARRAY;
        }
        assert(false);
        return GL_NONE;
    }

    inline GLenum ToGL(ImageFormat format)
    {
        switch (format)
        {
            using enum ImageFormat;
            case Red:
                return GL_RED;
            case RG:
                return GL_RG;
            case RGB:
                return GL_RGB;
            case RGBA:
                return GL_RGBA;
            case Unknown:
                break;
        }
        assert(false);
        return GL_NONE;
    }

    inline GLenum ToGL(ImageDataType type)
    {
        switch (type)
        {
            using enum ImageDataType;
            case UByte:
                return GL_UNSIGNED_BYTE;
            case Float:
                return GL_FLOAT;
            case Unknown:
                break;
        }
        assert(false);
        return GL_NONE;
    }

    inline GLenum ToGL(ImageAccess access)
    {
        switch (access)
        {
            using enum ImageAccess;
            case ReadOnly:
                return GL_READ_ONLY;
            case WriteOnly:
                return GL_WRITE_ONLY;
            case ReadWrite:
                return GL_READ_WRITE;
        }
        assert(false);
        return GL_NONE;
    }

    inline GLenum ToGL(BlendOp op)
    {
        switch (op)
        {
            using enum BlendOp;
            case Add:
                return GL_FUNC_ADD;
            case Subtract:
                return GL_FUNC_SUBTRACT;
            case ReverseSubtract:
                return GL_FUNC_REVERSE_SUBTRACT;
            case Min:
                return GL_MIN;
            case Max:
                return GL_MAX;
        }
        assert(false);
        return GL_NONE;
    }

    inline GLenum ToGL(BlendFactor factor)
    {
        switch (factor)
        {
            using enum BlendFactor;
            case Zero:
                return GL_ZERO;
            case One:
                return GL_ONE;
            case SrcColor:
                return GL_SRC_COLOR;
            case OneMinusSrcColor:
                return GL_ONE_MINUS_SRC_COLOR;
            case DstColor:
                return GL_DST_COLOR;
            case OneMinusDstColor:
                return GL_ONE_MINUS_DST_COLOR;
            case SrcAlpha:
                return GL_SRC_ALPHA;
            case OneMinusSrcAlpha:
                return GL_ONE_MINUS_SRC_ALPHA;
            case DstAlpha:
                return GL_DST_ALPHA;
            case OneMinusDstAlpha:
                return GL_ONE_MINUS_DST_ALPHA;
            case ConstantColor:
                return GL_CONSTANT_COLOR;
            case OneMinusConstantColor:
                return GL_ONE_MINUS_CONSTANT_COLOR;
            case ConstantAlpha:
                return GL_CONSTANT_ALPHA;
            case OneMinusConstantAlpha:
                return GL_ONE_MINUS_CONSTANT_ALPHA;
            case SrcAlphaSaturate:
                return GL_SRC_ALPHA_SATURATE;
            case Src1Color:
                return GL_SRC1_COLOR;
            case OneMinusSrc1Color:
                return GL_ONE_MINUS_SRC1_COLOR;
            case Src1Alpha:
                return GL_SRC1_ALPHA;
            case OneMinusSrc1Alpha:
                return GL_ONE_MINUS_SRC1_ALPHA;
        }
        assert(false);
        return GL_NONE;
    }

    inline GLenum ToGL(PolygonMode mode)
    {
        switch (mode)
        {
            using enum PolygonMode;
            case Point:
                return GL_POINT;
            case Line:
                return GL_LINE;
            case Fill:
                return GL_FILL;
        }
        assert(false);
        return GL_NONE;
    }

    inline GLenum ToGL(CullMode mode)
    {
        switch (mode)
        {
            using enum CullMode;
            case None:
                return GL_NONE;
            case Back:
                return GL_BACK;
            case Front:
                return GL_FRONT;
        }
        assert(false);
        return GL_NONE;
    }
} // namespace SnowLeopardEngine::GL
