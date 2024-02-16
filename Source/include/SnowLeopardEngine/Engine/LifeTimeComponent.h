#pragma once

namespace SnowLeopardEngine
{
    class LifeTimeComponent
    {
    public:
        virtual void OnLoad()                = 0;
        virtual void OnTick(float deltaTime) = 0;
        virtual void OnFixedTick()           = 0;
        virtual void OnUnload()              = 0;
    };
} // namespace SnowLeopardEngine
