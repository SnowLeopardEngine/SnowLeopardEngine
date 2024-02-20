#pragma once

#include "SnowLeopardEngine/Core/Base/Base.h"
#include "SnowLeopardEngine/Function/Rendering/Pass/RenderSubPass.h"

namespace SnowLeopardEngine
{
    class RenderPass
    {
    public:
        virtual ~RenderPass() = default;

        virtual void BeginPass() = 0;
        virtual void EndPass()   = 0;

        virtual void Draw() {}
    };
} // namespace SnowLeopardEngine