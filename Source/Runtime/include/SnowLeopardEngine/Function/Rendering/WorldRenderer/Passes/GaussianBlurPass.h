#pragma once

#include "SnowLeopardEngine/Function/Rendering/RenderContext.h"

#include <fg/Fwd.hpp>

namespace SnowLeopardEngine
{
    class GaussianBlurPass
    {
    public:
        explicit GaussianBlurPass(RenderContext&);
        ~GaussianBlurPass();

        FrameGraphResource AddToGraph(FrameGraph&, FrameGraphResource input, float scale);

    private:
        FrameGraphResource AddToGraph(FrameGraph&, FrameGraphResource input, float scale, bool horizontal);

    private:
        RenderContext&   m_RenderContext;
        GraphicsPipeline m_Pipeline;
    };
} // namespace SnowLeopardEngine