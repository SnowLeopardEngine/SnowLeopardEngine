#pragma once

#include "SnowLeopardEngine/Function/Rendering/RenderTypeDef.h"
#include "SnowLeopardEngine/Function/Rendering/Renderable.h"
#include "SnowLeopardEngine/Function/Rendering/WorldRenderer/Passes/BaseGeometryPass.h"
#include "SnowLeopardEngine/Function/Scene/Components.h"

#include <fg/Fwd.hpp>

namespace SnowLeopardEngine
{
    class ShadowPrePass : BaseGeometryPass
    {
    public:
        explicit ShadowPrePass(RenderContext& rc);
        ~ShadowPrePass();

        void BuildCascadedShadowMaps(FrameGraph&,
                                     FrameGraphBlackboard&,
                                     Entity cameraEntity,
                                     Entity lightEntity,
                                     std::span<Renderable>);

    protected:
        GraphicsPipeline CreateBasePassPipeline(const VertexFormat&, const Material* material) override final;

    private:
        FrameGraphResource AddCascadePass(FrameGraph&,
                                          FrameGraphBlackboard&,
                                          std::optional<FrameGraphResource> cascadedShadowMaps,
                                          const glm::mat4&                  lightViewProj,
                                          std::vector<Renderable>&&,
                                          uint32_t cascadeIdx);

    private:
        Buffer m_CascadedUniformBuffer;
    };
} // namespace SnowLeopardEngine