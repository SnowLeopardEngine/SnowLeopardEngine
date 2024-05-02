#pragma once

#include "SnowLeopardEngine/Function/Rendering/RenderTypeDef.h"
#include "SnowLeopardEngine/Function/Rendering/Renderable.h"
#include "SnowLeopardEngine/Function/Rendering/WorldRenderer/Passes/BaseGeometryPass.h"

#include <fg/Fwd.hpp>

namespace SnowLeopardEngine
{
    class GrassPass : BaseGeometryPass
    {
    public:
        explicit GrassPass(RenderContext& renderContext);
        ~GrassPass() = default;

        void AddToGraph(FrameGraph&, FrameGraphBlackboard&, const RenderableGroups& group);

    protected:
        GraphicsPipeline CreateBasePassPipeline(const VertexFormat&, const Material* material) override final;
    };
} // namespace SnowLeopardEngine