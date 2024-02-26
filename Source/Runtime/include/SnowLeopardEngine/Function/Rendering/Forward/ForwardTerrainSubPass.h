#pragma once

#include "SnowLeopardEngine/Function/Rendering/Pass/RenderSubDrawPass.h"

namespace SnowLeopardEngine
{
    class ForwardTerrainSubPass : public RenderSubDrawPass
    {
    public:
        virtual void BeginSubPass() override {}
        virtual void EndSubPass() override {}

        virtual void Draw() override final;
    };
} // namespace SnowLeopardEngine