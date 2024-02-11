#include "SnowLeopardEngine/Function/Window/WindowSystem.h"
#include "SnowLeopardEngine/Platform/GLFW/GLFWWindow.h"

namespace SnowLeopardEngine
{
    WindowSystem::WindowSystem(const WindowSystemInitInfo& props)
    {
        m_Window = new GLFWWindow();
        m_Window->Init(props.Window);
    }

    WindowSystem::~WindowSystem()
    {
        m_Window->Shutdown();
        delete m_Window;
    }
} // namespace SnowLeopardEngine
