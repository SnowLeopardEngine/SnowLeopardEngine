#pragma once

#include "SnowLeopardEngine/Function/Rendering/RenderTypeDef.h"
#include "SnowLeopardEngine/Function/Rendering/Renderable.h"
#include "SnowLeopardEngine/Function/Rendering/WorldRenderer/Passes/BaseGeometryPass.h"

#include <fg/Fwd.hpp>

namespace SnowLeopardEngine
{
    class WeightedBlendedPass : BaseGeometryPass
    {
    public:
        explicit WeightedBlendedPass(RenderContext& renderContext);
        ~WeightedBlendedPass() = default;

        void AddToGraph(FrameGraph& fg, FrameGraphBlackboard& blackboard, const RenderableGroups& groups);

    protected:
        GraphicsPipeline CreateBasePassPipeline(const VertexFormat&, const Material* material) override final;
    };
} // namespace SnowLeopardEngine