#pragma once

namespace SnowLeopardEngine::Editor
{
    class PanelBase
    {
    public:
        PanelBase()          = default;
        virtual ~PanelBase() = default;

        virtual void Init() {}
        virtual void OnTick(float deltaTime) {}
    };
} // namespace SnowLeopardEngine::Editor