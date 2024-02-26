#pragma once

#include "SnowLeopardEngine/Function/Rendering/Pass/RenderSubPass.h"

namespace SnowLeopardEngine
{
    class RenderSubDrawPass : public RenderSubPass
    {
    public:
        virtual void Draw() = 0;
    };
} // namespace SnowLeopardEngine