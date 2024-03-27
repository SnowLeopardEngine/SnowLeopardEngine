#include "SnowLeopardEngine/Platform/OpenGL/GLAPI.h"
#include "SnowLeopardEngine/Core/Profiling/Profiling.h"

#include "SnowLeopardEngine/Function/Rendering/Pipeline/PipelineState.h"
#include <glad/glad.h>

namespace SnowLeopardEngine
{
    void OpenGLAPI::SetPipelineState(const PipelineState& pipelineState)
    {
        SNOW_LEOPARD_PROFILE_FUNCTION
        switch (pipelineState.DepthTest)
        {
            case DepthTestMode::Disable:
                glDisable(GL_DEPTH_TEST);
                break;
            case DepthTestMode::Greater:
                glEnable(GL_DEPTH_TEST);
                glDepthFunc(GL_GREATER);
                break;
            case DepthTestMode::GreaterEqual:
                glEnable(GL_DEPTH_TEST);
                glDepthFunc(GL_GEQUAL);
                break;
            case DepthTestMode::Less:
                glEnable(GL_DEPTH_TEST);
                glDepthFunc(GL_LESS);
                break;
            case DepthTestMode::LessEqual:
                glEnable(GL_DEPTH_TEST);
                glDepthFunc(GL_LEQUAL);
                break;
            case DepthTestMode::Equal:
                glEnable(GL_DEPTH_TEST);
                glDepthFunc(GL_EQUAL);
                break;
            case DepthTestMode::NotEqual:
                glEnable(GL_DEPTH_TEST);
                glDepthFunc(GL_NOTEQUAL);
                break;
            case DepthTestMode::Never:
                glEnable(GL_DEPTH_TEST);
                glDepthFunc(GL_NEVER);
                break;
            case DepthTestMode::AlwaysPass:
                glEnable(GL_DEPTH_TEST);
                glDepthFunc(GL_ALWAYS);
                break;
            default:
                assert(0);
        }

        if (pipelineState.DepthWrite)
        {
            glDepthMask(GL_TRUE);
        }
        else
        {
            glDepthMask(GL_FALSE);
        }

        switch (pipelineState.CullFace)
        {
            case CullFaceMode::NoCull:
                glDisable(GL_CULL_FACE);
                break;
            case CullFaceMode::Front:
                glEnable(GL_CULL_FACE);
                glCullFace(GL_FRONT);
                break;
            case CullFaceMode::Back:
                glEnable(GL_CULL_FACE);
                glCullFace(GL_BACK);
                break;
            case CullFaceMode::Both:
                glEnable(GL_CULL_FACE);
                glCullFace(GL_FRONT_AND_BACK);
                break;
            default:
                assert(0);
        }

        switch (pipelineState.Blend)
        {
            case BlendMode::Disable:
                glDisable(GL_BLEND);
                break;

            case BlendMode::Enable:
                glEnable(GL_BLEND);
                break;

            default:
                assert(0);
        }

        if (pipelineState.Blend == BlendMode::Enable)
        {
            GLenum srcFactor, dstFactor;

            switch (pipelineState.BlendFunc1)
            {
                case BlendFunc::Zero:
                    srcFactor = GL_ZERO;
                    break;
                case BlendFunc::One:
                    srcFactor = GL_ONE;
                    break;
                case BlendFunc::SrcColor:
                    srcFactor = GL_SRC_COLOR;
                    break;
                case BlendFunc::OneMinusSrcColor:
                    srcFactor = GL_ONE_MINUS_SRC_COLOR;
                    break;
                case BlendFunc::DstColor:
                    srcFactor = GL_DST_COLOR;
                    break;
                case BlendFunc::OneMinusDstColor:
                    srcFactor = GL_ONE_MINUS_DST_COLOR;
                    break;
                case BlendFunc::SrcAlpha:
                    srcFactor = GL_SRC_ALPHA;
                    break;
                case BlendFunc::OneMinusSrcAlpha:
                    srcFactor = GL_ONE_MINUS_SRC_ALPHA;
                    break;
                case BlendFunc::DstAlpha:
                    srcFactor = GL_DST_ALPHA;
                    break;
                case BlendFunc::OneMinusDstAlpha:
                    srcFactor = GL_ONE_MINUS_DST_ALPHA;
                    break;
                case BlendFunc::ConstantColor:
                    srcFactor = GL_CONSTANT_COLOR;
                    break;
                case BlendFunc::OneMinusConstantColor:
                    srcFactor = GL_ONE_MINUS_CONSTANT_COLOR;
                    break;
                case BlendFunc::ConstantAlpha:
                    srcFactor = GL_CONSTANT_ALPHA;
                    break;
                case BlendFunc::OneMinusConstantAlpha:
                    srcFactor = GL_ONE_MINUS_CONSTANT_ALPHA;
                    break;
                default:
                    assert(0);
            }

            switch (pipelineState.BlendFunc2)
            {
                case BlendFunc::Zero:
                    dstFactor = GL_ZERO;
                    break;
                case BlendFunc::One:
                    dstFactor = GL_ONE;
                    break;
                case BlendFunc::SrcColor:
                    dstFactor = GL_SRC_COLOR;
                    break;
                case BlendFunc::OneMinusSrcColor:
                    dstFactor = GL_ONE_MINUS_SRC_COLOR;
                    break;
                case BlendFunc::DstColor:
                    dstFactor = GL_DST_COLOR;
                    break;
                case BlendFunc::OneMinusDstColor:
                    dstFactor = GL_ONE_MINUS_DST_COLOR;
                    break;
                case BlendFunc::SrcAlpha:
                    dstFactor = GL_SRC_ALPHA;
                    break;
                case BlendFunc::OneMinusSrcAlpha:
                    dstFactor = GL_ONE_MINUS_SRC_ALPHA;
                    break;
                case BlendFunc::DstAlpha:
                    dstFactor = GL_DST_ALPHA;
                    break;
                case BlendFunc::OneMinusDstAlpha:
                    dstFactor = GL_ONE_MINUS_DST_ALPHA;
                    break;
                case BlendFunc::ConstantColor:
                    dstFactor = GL_CONSTANT_COLOR;
                    break;
                case BlendFunc::OneMinusConstantColor:
                    dstFactor = GL_ONE_MINUS_CONSTANT_COLOR;
                    break;
                case BlendFunc::ConstantAlpha:
                    dstFactor = GL_CONSTANT_ALPHA;
                    break;
                case BlendFunc::OneMinusConstantAlpha:
                    dstFactor = GL_ONE_MINUS_CONSTANT_ALPHA;
                    break;
                default:
                    assert(0);
            }

            glBlendFunc(srcFactor, dstFactor);
        }
    }

    void OpenGLAPI::SetPipelineState(const Ref<PipelineState>& pipelineState)
    {
        SNOW_LEOPARD_PROFILE_FUNCTION
        switch (pipelineState->DepthTest)
        {
            case DepthTestMode::Disable:
                glDisable(GL_DEPTH_TEST);
                break;
            case DepthTestMode::Greater:
                glEnable(GL_DEPTH_TEST);
                glDepthFunc(GL_GREATER);
                break;
            case DepthTestMode::GreaterEqual:
                glEnable(GL_DEPTH_TEST);
                glDepthFunc(GL_GEQUAL);
                break;
            case DepthTestMode::Less:
                glEnable(GL_DEPTH_TEST);
                glDepthFunc(GL_LESS);
                break;
            case DepthTestMode::LessEqual:
                glEnable(GL_DEPTH_TEST);
                glDepthFunc(GL_LEQUAL);
                break;
            case DepthTestMode::Equal:
                glEnable(GL_DEPTH_TEST);
                glDepthFunc(GL_EQUAL);
                break;
            case DepthTestMode::NotEqual:
                glEnable(GL_DEPTH_TEST);
                glDepthFunc(GL_NOTEQUAL);
                break;
            case DepthTestMode::Never:
                glEnable(GL_DEPTH_TEST);
                glDepthFunc(GL_NEVER);
                break;
            case DepthTestMode::AlwaysPass:
                glEnable(GL_DEPTH_TEST);
                glDepthFunc(GL_ALWAYS);
                break;
            default:
                assert(0);
        }

        if (pipelineState->DepthWrite)
        {
            glDepthMask(GL_TRUE);
        }
        else
        {
            glDepthMask(GL_FALSE);
        }

        switch (pipelineState->CullFace)
        {
            case CullFaceMode::NoCull:
                glDisable(GL_CULL_FACE);
                break;
            case CullFaceMode::Front:
                glEnable(GL_CULL_FACE);
                glCullFace(GL_FRONT);
                break;
            case CullFaceMode::Back:
                glEnable(GL_CULL_FACE);
                glCullFace(GL_BACK);
                break;
            default:
                assert(0);
        }
    }

    void OpenGLAPI::ClearColor(float r, float g, float b, float a, ClearBit clearBit)
    {
        glClearColor(r, g, b, a);
        GLbitfield field = 0;
        if ((clearBit & ClearBit::Color) == ClearBit::Color)
        {
            field |= GL_COLOR_BUFFER_BIT;
        }
        if ((clearBit & ClearBit::Depth) == ClearBit::Depth)
        {
            field |= GL_DEPTH_BUFFER_BIT;
        }
        glClear(field);
    }

    void OpenGLAPI::ClearColor(const glm::vec4& color, ClearBit clearBit)
    {
        SNOW_LEOPARD_PROFILE_FUNCTION
        ClearColor(color.r, color.g, color.b, color.a, clearBit);
    }

    void OpenGLAPI::UpdateViewport(uint32_t x, uint32_t y, uint32_t width, uint32_t height)
    {
        SNOW_LEOPARD_PROFILE_FUNCTION
        glViewport(x, y, width, height);
    }

    ViewportDesc OpenGLAPI::GetViewport()
    {
        SNOW_LEOPARD_PROFILE_FUNCTION
        GLint viewport[4];
        glGetIntegerv(GL_VIEWPORT, viewport);

        ViewportDesc desc = {};
        desc.X            = viewport[0];
        desc.Y            = viewport[1];
        desc.Width        = viewport[2];
        desc.Height       = viewport[3];

        return desc;
    }

    Ref<VertexArray> OpenGLAPI::CreateVertexArray(const MeshItem& meshItem)
    {
        // Create a default layout for static meshes and set it
        BufferLayout layout = {{ShaderDataType::Float3, "a_Position"},
                               {ShaderDataType::Float3, "a_Normal"},
                               {ShaderDataType::Float2, "a_TexCoords"},
                               {ShaderDataType::Int, "a_EntityID"}};

        return CreateVertexArray(meshItem, layout);
    }

    Ref<VertexArray> OpenGLAPI::CreateVertexArray(const MeshItem& meshItem, const BufferLayout& inputLayout)
    {
        SNOW_LEOPARD_PROFILE_FUNCTION
        // Create the vertex array
        auto vertexArray = VertexArray::Create();

        // Create vertices and the vertex buffer
        auto vertexBuffer = VertexBuffer::Create(meshItem.Data.Vertices);

        vertexBuffer->SetLayout(inputLayout);

        // Create indices and the index buffer
        auto indices     = meshItem.Data.Indices;
        auto indexBuffer = IndexBuffer::Create(indices);

        vertexArray->AddBuffers(vertexBuffer, indexBuffer);

        return vertexArray;
    }

    void OpenGLAPI::DrawIndexed(uint32_t indexCount)
    {
        SNOW_LEOPARD_PROFILE_FUNCTION
        glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, nullptr);
    }

    void OpenGLAPI::DrawIndexedInstanced(uint32_t indexCount, uint32_t instanceCount)
    {
        SNOW_LEOPARD_PROFILE_FUNCTION
        glDrawElementsInstanced(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, nullptr, instanceCount);
    }

    bool OpenGLAPI::IsDSASupported() { return GLAD_GL_VERSION_4_5 || GLAD_GL_VERSION_4_6; }
} // namespace SnowLeopardEngine