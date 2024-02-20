#pragma once

#include "SnowLeopardEngine/Function/Rendering/Pass/RenderPass.h"

namespace SnowLeopardEngine
{
    class RenderDrawPass : public RenderPass
    {
    public:
        virtual void Draw() = 0;
    };
} // namespace SnowLeopardEngine