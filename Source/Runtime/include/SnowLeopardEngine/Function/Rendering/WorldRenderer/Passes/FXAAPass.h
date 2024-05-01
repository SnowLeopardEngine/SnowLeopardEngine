#pragma once

#include "SnowLeopardEngine/Function/Rendering/RenderContext.h"

#include <fg/Fwd.hpp>

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