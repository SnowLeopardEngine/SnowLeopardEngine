#include "SnowLeopardEngine/Function/Window/WindowSystem.h"
#include "SnowLeopardEngine/Platform/SDL3/SDL3Window.h"

#include <cstdlib>

namespace SnowLeopardEngine
{
    bool WindowInitInfo::IsOffline() const
    {
        if (const char* value = std::getenv("SLE_OFFLINE"); value != nullptr)
        {
            return value[0] != '0';
        }
        return Offline;
    }

    WindowSystem::WindowSystem(const WindowSystemInitInfo& props)
    {
        m_Offline = props.Window.IsOffline();
        m_Window  = new SDL3Window();
        m_Window->Init(props.Window);
    }

    WindowSystem::~WindowSystem()
    {
        m_Window->Shutdown();
        delete m_Window;
    }
} // namespace SnowLeopardEngine
