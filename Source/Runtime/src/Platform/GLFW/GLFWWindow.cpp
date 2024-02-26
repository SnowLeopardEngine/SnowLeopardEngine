#include "SnowLeopardEngine/Platform/GLFW/GLFWWindow.h"
#include "SnowLeopardEngine/Core/Event/EventUtil.h"
#include "SnowLeopardEngine/Core/Event/WindowEvents.h"
#include "SnowLeopardEngine/Engine/EngineContext.h"

namespace SnowLeopardEngine
{
    static uint8_t s_glfwWindowCount = 0;

    static void GLFWErrorCallback(int error, const char* description)
    {
        SNOW_LEOPARD_CORE_ERROR("[GLFWWindow] GLFW Error ({0}) {1}", error, description);
    }

    void GLFWWindow::Init(const WindowInitInfo& initInfo)
    {
        m_Data.Title     = initInfo.Title;
        m_Data.Width     = initInfo.Width;
        m_Data.Height    = initInfo.Height;
        m_Data.WindowSys = this;

        SNOW_LEOPARD_CORE_INFO("[GLFWWindow] Creating window, name: {0}, resolution: {1} x {2}",
                               initInfo.Title,
                               initInfo.Width,
                               initInfo.Height);

        if (s_glfwWindowCount == 0)
        {
            int result = glfwInit();
            assert(result);
            glfwSetErrorCallback(GLFWErrorCallback);
        }

        // Create window
#ifndef NDEBUG
        glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GLFW_TRUE);
#endif
        // High DPI
        glfwWindowHint(GLFW_SCALE_TO_MONITOR, GLFW_TRUE);

        glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_API);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, SNOW_LEOPARD_RENDER_API_OPENGL_MIN_MAJOR);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, SNOW_LEOPARD_RENDER_API_OPENGL_MIN_MINOR);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

        // MSAA
        glfwWindowHint(GLFW_SAMPLES, 4);

#if SNOW_LEOPARD_PLATFORM_DARWIN
        glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GLFW_TRUE);
#endif
        m_Window = glfwCreateWindow(static_cast<int>(initInfo.Width),
                                    static_cast<int>(initInfo.Height),
                                    m_Data.Title.c_str(),
                                    nullptr,
                                    nullptr);
        ++s_glfwWindowCount;

        // Update width and height when High DPI is enabled.
        int frameBufferWidth, frameBufferHeight;
        glfwGetFramebufferSize(m_Window, &frameBufferWidth, &frameBufferHeight);
        m_Data.Width  = frameBufferWidth;
        m_Data.Height = frameBufferHeight;

#if !SNOW_LEOPARD_PLATFORM_DARWIN
        // Get vid mode
        const GLFWvidmode* mode = glfwGetVideoMode(glfwGetPrimaryMonitor());

        int xPos = (mode->width - m_Data.Width) / 2;
        int yPos = (mode->height - m_Data.Height) / 2;

        // Set window default position (center of screen)
        glfwSetWindowPos(m_Window, xPos, yPos);
#endif

        // TODO: Set icon

        glfwSetWindowUserPointer(m_Window, &m_Data);

        // Set GLFW callbacks
        glfwSetWindowSizeCallback(m_Window, [](GLFWwindow* window, int width, int height) {
            WindowData& data = *static_cast<WindowData*>(glfwGetWindowUserPointer(window));
            data.Width       = width;
            data.Height      = height;

            // Trigger event
            WindowResizeEvent event(width, height);
            TriggerEvent(event);
        });

        glfwSetWindowCloseCallback(m_Window, [](GLFWwindow* window) {
            WindowData& data = *static_cast<WindowData*>(glfwGetWindowUserPointer(window));

            // Queue event
            Scope<WindowCloseEvent> event = std::move(CreateScope<WindowCloseEvent>());
            QueueEvent(std::move(event));
        });

        glfwSetKeyCallback(m_Window, [](GLFWwindow* window, int key, int scancode, int action, int mods) {
            WindowData& data = *static_cast<WindowData*>(glfwGetWindowUserPointer(window));
            g_EngineContext->InputSys->SetKeyState(key, action);
        });

        glfwSetCharCallback(m_Window, [](GLFWwindow* window, unsigned int keycode) {
            WindowData& data = *static_cast<WindowData*>(glfwGetWindowUserPointer(window));

            // TODO: Ruofan He InputSystem set keyboard states & dispatch event

            // TODO: Ruofan He InputSystem set keyboard states
        });

        glfwSetMouseButtonCallback(m_Window, [](GLFWwindow* window, int button, int action, int mods) {
            WindowData& data = *static_cast<WindowData*>(glfwGetWindowUserPointer(window));
            g_EngineContext->InputSys->SetMouseButtonState(button, action);
        });

        glfwSetScrollCallback(m_Window, [](GLFWwindow* window, double xOffset, double yOffset) {
            WindowData& data = *static_cast<WindowData*>(glfwGetWindowUserPointer(window));

            g_EngineContext->InputSys->SetMouseScrollDelta(glm::vec2(xOffset, yOffset));
        });

        glfwSetCursorPosCallback(m_Window, [](GLFWwindow* window, double xPos, double yPos) {
            WindowData& data = *static_cast<WindowData*>(glfwGetWindowUserPointer(window));

            g_EngineContext->InputSys->SetMousePosition(glm::vec2(xPos, yPos));
        });

        // TODO: Ruofan He InputSystem add Controller buttons & joysticks support.
        // https://www.glfw.org/docs/3.3/input_guide.html#joystick

        SNOW_LEOPARD_CORE_INFO("[GLFWWindow] Initialized");
    }

    void GLFWWindow::Shutdown()
    {
        SNOW_LEOPARD_CORE_INFO("[GLFWWindow] Shutting Down...");

        glfwDestroyWindow(m_Window);
        --s_glfwWindowCount;

        if (s_glfwWindowCount == 0)
        {
            glfwTerminate();
        }
    }

    bool GLFWWindow::OnTick()
    {
        g_EngineContext->InputSys->ClearStates();
        glfwPollEvents();
        return true;
    }

    bool GLFWWindow::ShouldClose() const { return glfwWindowShouldClose(m_Window); }

    void GLFWWindow::MakeCurrentContext() { glfwMakeContextCurrent(m_Window); }

    void GLFWWindow::SwapBuffers() { glfwSwapBuffers(m_Window); }

    void GLFWWindow::SetHideCursor(bool hide)
    {
        glfwSetInputMode(m_Window, GLFW_CURSOR, hide ? GLFW_CURSOR_DISABLED : GLFW_CURSOR_NORMAL);
    }
} // namespace SnowLeopardEngine
