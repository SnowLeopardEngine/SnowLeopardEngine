#pragma once

#include "SnowLeopardEngine/Core/Base/Base.h"
#include "SnowLeopardEngine/Core/Event/EventHandler.h"
#include "SnowLeopardEngine/Core/Event/WindowEvents.h"
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
        DesktopApp(int argc, char** argv);

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
        virtual bool PostInit();

        /**
         * @brief Let the application start running.
         *
         */
        virtual void Run();

        /**
         * @brief Quit the application.
         *
         */
        virtual void Quit();

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

    protected:
        /**
         * @brief Shutdown the application.
         *
         */
        virtual void Shutdown();

        /**
         * @brief Event callback when window is closing
         *
         */
        virtual void OnWindowClose(const WindowCloseEvent& e);

    protected:
        Ref<Engine> m_Engine = nullptr;
        Timer       m_Timer;
        bool        m_IsRunning = false;

        EventHandler<WindowCloseEvent> m_WindowCloseHandler = [this](const WindowCloseEvent& e) { OnWindowClose(e); };

        static DesktopApp* s_Instance;
    };
} // namespace SnowLeopardEngine
