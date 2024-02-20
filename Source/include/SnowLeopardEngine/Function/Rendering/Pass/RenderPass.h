#pragma once

namespace SnowLeopardEngine
{
    class RenderPass
    {
    public:
        virtual ~RenderPass() = default;

        virtual void BeginPass() = 0;
        virtual void EndPass()   = 0;
    };
} // namespace SnowLeopardEngine