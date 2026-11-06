#pragma once

#include "SnowLeopardEngine/Function/Rendering/RenderContext.h"

#include <fg/Fwd.hpp>

#include <optional>

namespace SnowLeopardEngine
{
    class FinalPass
    {
    public:
        explicit FinalPass(RenderContext&);
        ~FinalPass();

        // Draws the scene colour to the window, or into `target` (an imported texture) when given.
        void Compose(FrameGraph&, const FrameGraphBlackboard&, std::optional<FrameGraphResource> target = {});

    private:
        RenderContext&   m_RenderContext;
        GraphicsPipeline m_Pipeline;
    };
} // namespace SnowLeopardEngine