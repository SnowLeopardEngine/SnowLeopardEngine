#include "SnowLeopardEditor/Panels/ConsolePanel.h"
#include "SnowLeopardEngine/Engine/EngineContext.h"

namespace SnowLeopardEngine::Editor
{
    void ConsolePanel::Init()
    {
        // Register log event
        Subscribe(m_LogEventHandler);
    }

    void ConsolePanel::OnTick(float deltaTime)
    {
        // TODO: Yanni Ma, draw the console panel (mainly the messages from your defined container)
    }

    void ConsolePanel::Shutdown()
    {
        // Unregister log event
        Unsubscribe(m_LogEventHandler);
    }

    void ConsolePanel::OnLog(const LogEvent& e)
    {
        std::cout << fmt::format("\t\t\tReceive Log Event: {0}", e.ToString()) << std::endl;

        // TODO: Yanni Ma, use a container to store the log messages.
        // You can get LogLevel and Message from `e`.
        // e.GetRegion() (Core/Client) e.GetLevel() (Trace/Info/Warn/Error/Critical), e.GetMessage()
    }
} // namespace SnowLeopardEngine::Editor