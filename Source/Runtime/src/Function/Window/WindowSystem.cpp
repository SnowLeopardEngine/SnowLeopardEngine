#include "SnowLeopardEngine/Function/Window/WindowSystem.h"
#include "SnowLeopardEngine/Platform/SDL3/SDL3Window.h"

namespace SnowLeopardEngine
{
    WindowSystem::WindowSystem(const WindowSystemInitInfo& props)
    {
        m_Window = new SDL3Window();
        m_Window->Init(props.Window);
    }

    WindowSystem::~WindowSystem()
    {
        m_Window->Shutdown();
        delete m_Window;
    }
} // namespace SnowLeopardEngine
