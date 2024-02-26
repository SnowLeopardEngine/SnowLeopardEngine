#pragma once

#include "SnowLeopardEngine/Function/Rendering/Pass/RenderPass.h"

namespace SnowLeopardEngine
{
    class RenderDispatchPass : public RenderPass
    {
    public:
        virtual void Dispatch() = 0;
    };
} // namespace SnowLeopardEngine