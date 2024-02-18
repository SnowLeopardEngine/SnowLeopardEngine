#pragma once

namespace SnowLeopardEngine
{
    class GraphicsContext
    {
    public:
        virtual void Init()     = 0;
        virtual void Shutdown() = 0;

        virtual void SwapBuffers()               = 0;
        virtual void SetVSync(bool vsyncEnabled) = 0;
    };
} // namespace SnowLeopardEngine