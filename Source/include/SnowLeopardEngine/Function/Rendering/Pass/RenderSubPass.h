#pragma once

namespace SnowLeopardEngine
{
    class RenderSubPass
    {
    public:
        virtual ~RenderSubPass() = default;

        virtual void BeginSubPass() = 0;
        virtual void EndSubPass()   = 0;
    };
} // namespace SnowLeopardEngine