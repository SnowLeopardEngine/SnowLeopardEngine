#pragma once

#include "SnowLeopardEngine/Core/Base/Base.h"
#include "SnowLeopardEngine/Core/Time/Timer.h"
#include "SnowLeopardEngine/Engine/Engine.h"

namespace SnowLeopardEngine
{
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
        bool Init();

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
        static DesktopApp* Get() { return s_Instance; }

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
        void OnWindowClose();

    private:
        Ref<Engine> m_Engine = nullptr;
        Timer       m_Timer;
        bool        m_IsRunning = false;

        static DesktopApp* s_Instance;
    };
} // namespace SnowLeopardEngine
