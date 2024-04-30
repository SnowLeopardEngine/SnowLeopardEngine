#pragma once

#include "SnowLeopardEngine/Function/Rendering/Pipeline/GraphicsPipeline.h"
#include "SnowLeopardEngine/Function/Rendering/RenderContext.h"
#include "SnowLeopardEngine/Function/Rendering/RenderTypeDef.h"
#include "SnowLeopardEngine/Function/Rendering/Renderable.h"
#include "SnowLeopardEngine/Function/Rendering/WorldRenderer/Passes/BaseGeometryPass.h"

#include <fg/Fwd.hpp>

namespace SnowLeopardEngine
{
    class GBufferPass : BaseGeometryPass
    {
    public:
        explicit GBufferPass(RenderContext& renderContext);
        ~GBufferPass() = default;

        void AddToGraph(FrameGraph&             fg,
                        FrameGraphBlackboard&   blackboard,
                        const Extent2D&         resolution,
                        const RenderableGroups& group);

    protected:
        GraphicsPipeline CreateBasePassPipeline(const VertexFormat&, const Material* material) override final;
    };
} // namespace SnowLeopardEngine