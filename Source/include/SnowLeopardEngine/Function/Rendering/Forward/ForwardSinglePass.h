#pragma once

#include "SnowLeopardEngine/Function/Rendering/Pass/BaseRenderDrawPass.h"

namespace SnowLeopardEngine
{
    class ForwardSinglePass : public BaseRenderDrawPass
    {
    public:
        virtual void BeginPass() override final;
    };
} // namespace SnowLeopardEngine