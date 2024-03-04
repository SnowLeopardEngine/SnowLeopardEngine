#pragma once

namespace SnowLeopardEngine::Editor
{
    class PanelBase
    {
    public:
        PanelBase()          = default;
        virtual ~PanelBase() = default;

        virtual void Init() {}
        virtual void OnFixedTick() {}
        virtual void OnTick(float deltaTime) {}
        virtual void Shutdown() {}
    };
} // namespace SnowLeopardEngine::Editor