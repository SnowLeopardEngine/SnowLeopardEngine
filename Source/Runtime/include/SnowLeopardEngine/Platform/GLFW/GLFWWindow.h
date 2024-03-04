#pragma once

#include "SnowLeopardEngine/Function/Window/Window.h"
#include "SnowLeopardEngine/Platform/Platform.h"

#include <GLFW/glfw3.h>
#if SNOW_LEOPARD_PLATFORM_LINUX
#define GLFW_EXPOSE_NATIVE_X11
#elif SNOW_LEOPARD_PLATFORM_WINDOWS
#define GLFW_EXPOSE_NATIVE_WIN32
#elif SNOW_LEOPARD_PLATFORM_DARWIN
#define GLFW_EXPOSE_NATIVE_COCOA
#endif
#include <GLFW/glfw3native.h>

namespace SnowLeopardEngine
{
    class GLFWWindow final : public Window
    {
    public:
        virtual WindowType GetType() override { return WindowType::GLFW; }

        virtual void Init(const WindowInitInfo& initInfo) override;

        virtual void Shutdown() override;

        virtual bool OnTick() override;

        virtual unsigned int GetWidth() const override { return m_Data.Width; }

        virtual unsigned int GetHeight() const override { return m_Data.Height; }

        virtual bool ShouldClose() const override;
        virtual bool IsMinimized() const override;

        virtual void MakeCurrentContext() override;
        virtual void SwapBuffers() override;

        virtual void SetHideCursor(bool hide) override;

        virtual void* GetPlatformWindow() const override { return m_Window; }

        virtual void* GetNativeWindow() const override
        {
#if SNOW_LEOPARD_PLATFORM_LINUX
            return (void*)(uintptr_t)glfwGetX11Window(m_Window);
#elif SNOW_LEOPARD_PLATFORM_WINDOWS
            return glfwGetWin32Window(m_Window);
#elif SNOW_LEOPARD_PLATFORM_DARWIN
            return glfwGetCocoaWindow(m_Window);
#endif
        }

    private:
        GLFWwindow* m_Window;

        struct WindowData
        {
            std::string  Title;
            unsigned int Width, Height;
            GLFWWindow*  WindowSys;
            bool         IsMinimized;
        };

        WindowData m_Data;
    };
} // namespace SnowLeopardEngine