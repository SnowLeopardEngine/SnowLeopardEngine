#pragma once

#include "SnowLeopardEngine/Function/Rendering/Pipeline/GraphicsPipeline.h"
#include "SnowLeopardEngine/Function/Rendering/RenderContext.h"

#include <fg/Fwd.hpp>

namespace SnowLeopardEngine
{
    class BlitUIPass
    {
    public:
        explicit BlitUIPass(RenderContext&);
        ~BlitUIPass();

        FrameGraphResource AddToGraph(FrameGraph&, FrameGraphResource target, FrameGraphResource source);

    private:
        RenderContext&   m_RenderContext;
        GraphicsPipeline m_Pipeline;
    };
} // namespace SnowLeopardEngine