#pragma once

#include "SnowLeopardEditor/EditorGUISystem.h"
#include "SnowLeopardEngine/Engine/DesktopApp.h"

namespace SnowLeopardEngine::Editor
{
    struct EditorAppInitInfo
    {
        EngineInitInfo Engine;
    };

    class EditorApp : public DesktopApp
    {
    public:
        EditorApp(int argc, char** argv);

        /**
         * @brief Initialize the application.
         *
         * @return true no error
         * @return false has some errors
         */
        bool Init(const EditorAppInitInfo& initInfo);

        /**
         * @brief Post initialize the application
         *
         * @return true
         * @return false
         */
        virtual bool PostInit() override;

        /**
         * @brief Let the application start running.
         *
         */
        virtual void Run() override;

        /**
         * @brief Get global unique application instance.
         *
         * @return EditorApp* application instance
         */
        static EditorApp* GetInstance() { return s_Instance; }

    private:
        /**
         * @brief Shutdown the application.
         *
         */
        virtual void Shutdown() override;

    private:
        EditorGUISystem m_GUISystem;

        static EditorApp* s_Instance;
    };
} // namespace SnowLeopardEngine::Editor