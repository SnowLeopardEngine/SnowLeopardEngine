#pragma once

#include "SnowLeopardEngine/Function/Rendering/RenderContext.h"

#include <fg/Fwd.hpp>

namespace SnowLeopardEngine
{
    class SkyboxPass
    {
    public:
        explicit SkyboxPass(RenderContext&);
        ~SkyboxPass();

        FrameGraphResource AddToGraph(FrameGraph&, FrameGraphBlackboard&, FrameGraphResource target, Texture* cubemap);

    private:
        RenderContext&   m_RenderContext;
        GraphicsPipeline m_Pipeline;
    };
} // namespace SnowLeopardEngine