#pragma once

#include "SnowLeopardEngine/Function/Rendering/GraphicsAPI.h"

namespace SnowLeopardEngine
{
    class OpenGLAPI final : public GraphicsAPI
    {
    public:
        virtual GraphicsBackend GetBackendType() override final { return GraphicsBackend::OpenGL; }

        virtual void SetPipelineState(const Ref<PipelineState>& pipelineState) override final;

        virtual void ClearColor(float r, float g, float b, float a) override final;
        virtual void ClearColor(const glm::vec4& color) override final;

        virtual void         UpdateViewport(uint32_t x, uint32_t y, uint32_t width, uint32_t height) override final;
        virtual ViewportDesc GetViewport() override final;

        virtual Ref<VertexArray> CreateVertexArray(const MeshItem& meshItem) override final;

        virtual void DrawIndexed(uint32_t indexCount) override final;
    };
} // namespace SnowLeopardEngine