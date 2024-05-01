#pragma once

#include "SnowLeopardEngine/Function/Rendering/Pipeline/GraphicsPipeline.h"
#include "SnowLeopardEngine/Function/Rendering/RenderContext.h"

#include <fg/Fwd.hpp>

namespace SnowLeopardEngine
{
    class DownsamplePass
    {
    public:
        explicit DownsamplePass(RenderContext&);
        ~DownsamplePass();

        FrameGraphResource AddToGraph(FrameGraph&, FrameGraphResource input, uint32_t mipLevel);

    private:
        RenderContext&   m_RenderContext;
        GraphicsPipeline m_Pipeline;
    };
} // namespace SnowLeopardEngine