#pragma once

#include "SnowLeopardEngine/Function/Rendering/RenderContext.h"

#include <fg/Fwd.hpp>

namespace SnowLeopardEngine
{
    class ToneMappingPass
    {
    public:
        explicit ToneMappingPass(RenderContext&);
        ~ToneMappingPass();

        FrameGraphResource AddToGraph(FrameGraph&, FrameGraphResource input);

    private:
        RenderContext&   m_RenderContext;
        GraphicsPipeline m_Pipeline;
    };
} // namespace SnowLeopardEngine