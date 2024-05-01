#pragma once

#include "SnowLeopardEngine/Function/Rendering/Pipeline/GraphicsPipeline.h"
#include "SnowLeopardEngine/Function/Rendering/RenderContext.h"

#include <fg/Fwd.hpp>

namespace SnowLeopardEngine
{
    class UpsamplePass
    {
    public:
        explicit UpsamplePass(RenderContext&);
        ~UpsamplePass();

        FrameGraphResource AddToGraph(FrameGraph&, FrameGraphResource input, float radius);

    private:
        RenderContext&   m_RenderContext;
        GraphicsPipeline m_Pipeline;
    };
} // namespace SnowLeopardEngine