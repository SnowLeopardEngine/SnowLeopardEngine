#pragma once

namespace SnowLeopardEngine
{
    struct PhysicsMaterial
    {
        float DynamicFriction = 0;
        float StaticFriction  = 0;
        float Bounciness      = 0;

        PhysicsMaterial() = default;
        PhysicsMaterial(float dynamicFriction, float staticFriction, float bounciness) :
            DynamicFriction(dynamicFriction), StaticFriction(staticFriction), Bounciness(bounciness)
        {}
    };
} // namespace SnowLeopardEngine