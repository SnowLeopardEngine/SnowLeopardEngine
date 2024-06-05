#pragma once

#include "SnowLeopardEngine/Function/Rendering/RenderContext.h"

#include <fg/Fwd.hpp>

namespace SnowLeopardEngine
{
    class DeferredLightingPass
    {
    public:
        explicit DeferredLightingPass(RenderContext&);
        ~DeferredLightingPass();

        FrameGraphResource AddToGraph(FrameGraph&, FrameGraphBlackboard&);

    private:
        RenderContext&   m_RenderContext;
        GraphicsPipeline m_Pipeline;
    };
} // namespace SnowLeopardEngine