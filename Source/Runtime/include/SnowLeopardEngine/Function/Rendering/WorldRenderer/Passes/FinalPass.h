#pragma once

#include "SnowLeopardEngine/Function/Rendering/Pipeline/GraphicsPipeline.h"
#include "SnowLeopardEngine/Function/Rendering/RenderContext.h"

#include <fg/Fwd.hpp>

namespace SnowLeopardEngine
{
    class FinalPass
    {
    public:
        explicit FinalPass(RenderContext&);
        ~FinalPass();

        void Compose(FrameGraph&, const FrameGraphBlackboard&);

    private:
        RenderContext&   m_RenderContext;
        GraphicsPipeline m_Pipeline;
    };
} // namespace SnowLeopardEngine