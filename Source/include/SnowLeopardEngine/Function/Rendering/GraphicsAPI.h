#pragma once

#include "SnowLeopardEngine/Function/Rendering/Pipeline/PipelineState.h"
#include "SnowLeopardEngine/Function/Rendering/RHI/VertexArray.h"
#include "SnowLeopardEngine/Function/Rendering/RenderTypeDef.h"

namespace SnowLeopardEngine
{
    enum class GraphicsBackend
    {
        OpenGL = 0,
    };

    class GraphicsAPI
    {
    public:
        virtual GraphicsBackend GetBackendType() = 0;

        virtual void SetPipelineState(const Ref<PipelineState>& pipelineState) = 0;

        virtual void ClearColor(float r, float g, float b, float a) = 0;
        virtual void ClearColor(const glm::vec4& color)             = 0;

        virtual void         UpdateViewport(uint32_t x, uint32_t y, uint32_t width, uint32_t height) = 0;
        virtual ViewportDesc GetViewport()                                                           = 0;

        virtual Ref<VertexArray> CreateVertexArray(const MeshItem& meshItem) = 0;

        virtual void DrawIndexed(uint32_t indexCount) = 0;

        static Ref<GraphicsAPI> Create(GraphicsBackend backend);
    };
} // namespace SnowLeopardEngine