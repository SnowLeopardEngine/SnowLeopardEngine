#include "SnowLeopardEngine/Platform/OpenGL/GLAPI.h"

#include <glad/glad.h>

namespace SnowLeopardEngine
{
    void OpenGLAPI::SetPipelineState(const Ref<PipelineState>& pipelineState)
    {
        switch (pipelineState->DepthTest)
        {
            case DepthTestMode::None:
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
            case DepthTestMode::Always:
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
            case CullFaceMode::None:
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
        ClearColor(color.r, color.g, color.b, color.a, clearBit);
    }

    void OpenGLAPI::UpdateViewport(uint32_t x, uint32_t y, uint32_t width, uint32_t height)
    {
        glViewport(x, y, width, height);
    }

    ViewportDesc OpenGLAPI::GetViewport()
    {
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
        // Create a default layout and set it
        BufferLayout layout = {{ShaderDataType::Float3, "a_Position"},
                               {ShaderDataType::Float3, "a_Normal"},
                               {ShaderDataType::Float2, "a_TexCoords"},
                               {ShaderDataType::Int4, "a_BoneIDs"},
                               {ShaderDataType::Float4, "a_Weights"}};

        return CreateVertexArray(meshItem, layout);
    }

    Ref<VertexArray> OpenGLAPI::CreateVertexArray(const MeshItem& meshItem, const BufferLayout& inputLayout)
    {
        // Create the vertex array
        auto vertexArray = VertexArray::Create();

        // Create vertices and the vertex buffer
        auto vertices     = meshItem.Data.Vertices;
        auto vertexBuffer = VertexBuffer::Create(vertices);

        vertexBuffer->SetLayout(inputLayout);

        // Create indices and the index buffer
        auto indices     = meshItem.Data.Indices;
        auto indexBuffer = IndexBuffer::Create(indices);

        vertexArray->AddBuffers(vertexBuffer, indexBuffer);

        return vertexArray;
    }

    void OpenGLAPI::DrawIndexed(uint32_t indexCount)
    {
        glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, nullptr);
    }
} // namespace SnowLeopardEngine