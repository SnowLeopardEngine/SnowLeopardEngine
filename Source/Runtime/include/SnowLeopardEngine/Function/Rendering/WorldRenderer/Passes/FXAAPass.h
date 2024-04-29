#pragma once

#include "SnowLeopardEngine/Function/Rendering/Pipeline/GraphicsPipeline.h"
#include "SnowLeopardEngine/Function/Rendering/RenderContext.h"

#include <fg/Blackboard.hpp>
#include <fg/FrameGraph.hpp>

namespace SnowLeopardEngine
{
    class FXAAPass
    {
    public:
        explicit FXAAPass(RenderContext&);
        ~FXAAPass();

        FrameGraphResource AddToGraph(FrameGraph&, FrameGraphResource input);

    private:
        RenderContext&   m_RenderContext;
        GraphicsPipeline m_Pipeline;
    };
} // namespace SnowLeopardEngine