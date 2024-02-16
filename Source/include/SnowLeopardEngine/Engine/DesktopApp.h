#pragma once

#include "SnowLeopardEngine/Core/Base/Base.h"
#include "SnowLeopardEngine/Core/Event/ApplicationEvents.h"
#include "SnowLeopardEngine/Core/Event/EventHandler.h"
#include "SnowLeopardEngine/Core/Time/Timer.h"
#include "SnowLeopardEngine/Engine/Engine.h"

namespace SnowLeopardEngine
{
    struct DesktopAppInitInfo
    {
        EngineInitInfo Engine;
    };

    class DesktopApp
    {
    public:
        DesktopApp();

        /**
         * @brief Initialize the application.
         *
         * @return true no error
         * @return false has some errors
         */
        bool Init(const DesktopAppInitInfo& initInfo);

        /**
         * @brief Post initialize the application
         *
         * @return true
         * @return false
         */
        bool PostInit();

        /**
         * @brief Let the application start running.
         *
         */
        void Run();

        /**
         * @brief Quit the application.
         *
         */
        void Quit();

        /**
         * @brief Get the Engine
         *
         * @return Ref<Engine> the Engine
         */
        Ref<Engine> GetEngine() { return m_Engine; }

        /**
         * @brief Get global unique application instance.
         *
         * @return DesktopApp* application instance
         */
        static DesktopApp* GetInstance() { return s_Instance; }

    private:
        /**
         * @brief Shutdown the application.
         *
         */
        void Shutdown();

        /**
         * @brief Event callback when window is closing
         *
         */
        void OnWindowClose(const WindowCloseEvent& e);

    private:
        Ref<Engine> m_Engine = nullptr;
        Timer       m_Timer;
        bool        m_IsRunning = false;

        EventHandler<WindowCloseEvent> m_WindowCloseHandler = [this](const WindowCloseEvent& e) { OnWindowClose(e); };

        static DesktopApp* s_Instance;
    };
} // namespace SnowLeopardEngine
