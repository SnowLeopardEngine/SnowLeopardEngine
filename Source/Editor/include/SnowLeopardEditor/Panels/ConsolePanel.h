#pragma once

#include "SnowLeopardEditor/PanelBase.h"
#include "SnowLeopardEngine/Core/Event/EventHandler.h"
#include "SnowLeopardEngine/Core/Event/LogEvents.h"

namespace SnowLeopardEngine::Editor
{
    class ConsolePanel : public PanelBase
    {
    public:
        virtual void Init() override final;
        virtual void OnTick(float deltaTime) override final;
        virtual void Shutdown() override final;

    private:
        void OnLog(const LogEvent& e);

        EventHandler<LogEvent> m_LogEventHandler = [this](const LogEvent& e) { OnLog(e); };
    };
} // namespace SnowLeopardEngine::Editor