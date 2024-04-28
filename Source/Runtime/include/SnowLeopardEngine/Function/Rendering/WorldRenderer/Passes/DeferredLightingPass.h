#pragma once

#include "SnowLeopardEngine/Function/Rendering/Pipeline/GraphicsPipeline.h"
#include "SnowLeopardEngine/Function/Rendering/RenderContext.h"

#include <fg/Blackboard.hpp>
#include <fg/FrameGraph.hpp>

namespace SnowLeopardEngine
{
    class DeferredLightingPass
    {
    public:
        explicit DeferredLightingPass(RenderContext&);
        ~DeferredLightingPass();

        void AddToGraph(FrameGraph&, FrameGraphBlackboard&);

    private:
        RenderContext&   m_RenderContext;
        GraphicsPipeline m_Pipeline;
    };
} // namespace SnowLeopardEngine