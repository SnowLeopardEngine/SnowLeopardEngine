#include "SnowLeopardEngine/Platform/OpenGL/GLAPI.h"

#include <glad/glad.h>

namespace SnowLeopardEngine
{
    void OpenGLAPI::SetPipelineState(const Ref<PipelineState>& pipelineState)
    {
        // TODO: Set GL states
    }

    void OpenGLAPI::ClearColor(float r, float g, float b, float a)
    {
        glClearColor(r, g, b, a);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    }

    void OpenGLAPI::ClearColor(const glm::vec4& color) { ClearColor(color.r, color.g, color.b, color.a); }

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
        // Create the vertex array
        auto vertexArray = VertexArray::Create();

        // Create vertices and the vertex buffer
        auto vertices     = meshItem.Data.Vertices;
        auto vertexBuffer = VertexBuffer::Create(vertices);

        // Create layout and set it
        BufferLayout layout = {{ShaderDataType::Float3, "a_Position"},
                               {ShaderDataType::Float3, "a_Normal"},
                               {ShaderDataType::Float2, "a_TexCoords"}};
        vertexBuffer->SetLayout(layout);

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