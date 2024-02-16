#pragma once

namespace SnowLeopardEngine
{
    class LifeTimeComponent
    {
    public:
        virtual void OnLoad() {};
        virtual void OnTick(float deltaTime) {};
        virtual void OnFixedTick() {};
        virtual void OnUnload() {};
    };
} // namespace SnowLeopardEngine
