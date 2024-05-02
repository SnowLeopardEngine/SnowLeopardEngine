#pragma once

#include "SnowLeopardEngine/Function/Rendering/RenderContext.h"

#include <fg/Fwd.hpp>

namespace SnowLeopardEngine
{
    class TransparencyComposePass
    {
    public:
        explicit TransparencyComposePass(RenderContext&);
        ~TransparencyComposePass();

        FrameGraphResource AddToGraph(FrameGraph&, FrameGraphBlackboard&, FrameGraphResource target);

    private:
        RenderContext&   m_RenderContext;
        GraphicsPipeline m_Pipeline;
    };
} // namespace SnowLeopardEngine