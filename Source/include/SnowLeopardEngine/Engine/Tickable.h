#pragma once

namespace SnowLeopardEngine
{
    class Tickable
    {
    public:
        virtual void OnTick(float deltaTime) = 0;
        virtual void OnFixedTick()           = 0;
    };
} // namespace SnowLeopardEngine
