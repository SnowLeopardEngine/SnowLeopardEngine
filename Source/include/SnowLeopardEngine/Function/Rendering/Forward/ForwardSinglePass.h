#pragma once

#include "SnowLeopardEngine/Function/Rendering/Pass/RenderDrawPass.h"

namespace SnowLeopardEngine
{
    class ForwardSinglePass : public RenderDrawPass
    {
    public:
        virtual void BeginPass() override {}
        virtual void EndPass() override {}

        virtual void Draw() override final;
    };
} // namespace SnowLeopardEngine