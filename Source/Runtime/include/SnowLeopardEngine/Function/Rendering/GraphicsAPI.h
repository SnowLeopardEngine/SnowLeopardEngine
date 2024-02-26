#pragma once

#include "SnowLeopardEngine/Function/Rendering/Pipeline/PipelineState.h"
#include "SnowLeopardEngine/Function/Rendering/RHI/VertexArray.h"
#include "SnowLeopardEngine/Function/Rendering/RHI/VertexBuffer.h"
#include "SnowLeopardEngine/Function/Rendering/RenderTypeDef.h"

namespace SnowLeopardEngine
{
    enum class GraphicsBackend
    {
        OpenGL = 0,
    };

    enum class ClearBit : uint32_t
    {
        None    = 0,
        Color   = 1 << 0,
        Depth   = 1 << 1,
        Default = Color | Depth
    };
    FLAG_ENUM(ClearBit, uint32_t)

    class GraphicsAPI
    {
    public:
        virtual GraphicsBackend GetBackendType() = 0;

        virtual void SetPipelineState(const Ref<PipelineState>& pipelineState) = 0;

        virtual void ClearColor(float r, float g, float b, float a, ClearBit clearBit) = 0;
        virtual void ClearColor(const glm::vec4& color, ClearBit clearBit)             = 0;

        virtual void         UpdateViewport(uint32_t x, uint32_t y, uint32_t width, uint32_t height) = 0;
        virtual ViewportDesc GetViewport()                                                           = 0;

        virtual Ref<VertexArray> CreateVertexArray(const MeshItem& meshItem)                                  = 0;
        virtual Ref<VertexArray> CreateVertexArray(const MeshItem& meshItem, const BufferLayout& inputLayout) = 0;

        virtual void DrawIndexed(uint32_t indexCount) = 0;

        static Ref<GraphicsAPI> Create(GraphicsBackend backend);
    };
} // namespace SnowLeopardEngine