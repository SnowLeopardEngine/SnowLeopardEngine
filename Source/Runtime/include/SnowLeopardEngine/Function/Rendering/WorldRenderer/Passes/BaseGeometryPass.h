#pragma once

#include "SnowLeopardEngine/Function/Rendering/RenderContext.h"
#include "SnowLeopardEngine/Function/Rendering/VertexFormat.h"

namespace SnowLeopardEngine
{
    class Material;

    class BaseGeometryPass
    {
    public:
        explicit BaseGeometryPass(RenderContext&);
        virtual ~BaseGeometryPass();

    protected:
        void SetTransform(const glm::mat4& modelMatrix);

        GraphicsPipeline&        GetPipeline(const VertexFormat&, const Material* material);
        virtual GraphicsPipeline CreateBasePassPipeline(const VertexFormat&, const Material* material) = 0;

    protected:
        RenderContext&                               m_RenderContext;
        std::unordered_map<size_t, GraphicsPipeline> m_Pipelines;
    };

} // namespace SnowLeopardEngine