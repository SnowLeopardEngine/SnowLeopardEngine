#pragma once

#include "SnowLeopardEngine/Engine/LifeTimeComponent.h"

namespace SnowLeopardEngine
{
    class NativeScriptInstance : public LifeTimeComponent
    {
    public:
        virtual void OnLoad() override {}

        virtual void OnTick(float deltaTime) override {}

        virtual void OnFixedTick() override {}

        virtual void OnUnload() override {}

        virtual void OnColliderEnter() {}
    };
} // namespace SnowLeopardEngine
