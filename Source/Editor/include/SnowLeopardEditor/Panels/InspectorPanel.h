#pragma once

#include "SnowLeopardEditor/PanelBase.h"

namespace SnowLeopardEngine::Editor
{
    class InspectorPanel : public PanelBase
    {
    public:
        virtual void Init() override final;
        virtual void OnTick(float deltaTime) override final;
        virtual void Shutdown() override final;
    };
} // namespace SnowLeopardEngine::Editor