#include "SnowLeopardEditor/EditorApp.h"
#include "SnowLeopardEditor/EditorGUISystem.h"
#include "SnowLeopardEngine/Core/Time/Time.h"
#include "SnowLeopardEngine/Engine/DesktopApp.h"
#include "SnowLeopardEngine/Engine/EngineContext.h"
#include "SnowLeopardEngine/Function/IO/Resources.h"

namespace SnowLeopardEngine::Editor
{
    EditorApp* EditorApp::s_Instance = nullptr;

    EditorApp::EditorApp(int argc, char** argv) : DesktopApp(argc, argv)
    {
        m_Program.add_argument("--project").help("the .dzproj file").required();
        m_Program.parse_args(argc, argv);
        s_Instance = this;
    }

    bool EditorApp::Init(const EditorAppInitInfo& initInfo)
    {
        // handle CLI arguments
        auto project = m_Program.get("--project");
        std::cout << "Handling CLI argument: --project = " << project << std::endl;
        if (!std::filesystem::exists(project))
        {
            std::cerr << "Project file doesn't exist: " << project << std::endl;
            return false;
        }

        // create the engine
        m_Engine = CreateRef<Engine>();
        if (!m_Engine->Init(initInfo.Engine))
        {
            std::cerr << "Failed to initialize the engine!" << std::endl;
            return false;
        }

        // load the project
        auto loadingProject = g_EngineContext->ProjectMngr->CreateProject();
        if (!IO::Deserialize(loadingProject.get(), project))
        {
            std::cerr << "Failed to deserialize the project from: " << project << std::endl;
            return false;
        }
        loadingProject->SetPath(project);
        loadingProject->LoadAssets();
        g_EngineContext->ProjectMngr->SetActiveProject(loadingProject);

        // init GUI system
        EditorGUISystemInitInfo guiInitInfo = {};
        guiInitInfo.ProjectFilePath         = project;
        m_GUISystem.Init(guiInitInfo);

        // subscribe events
        Subscribe(m_WindowCloseHandler);

        m_IsRunning = true;

        SNOW_LEOPARD_INFO("[Editor] Initialized");

        return true;
    }

    bool EditorApp::PostInit()
    {
        if (!m_Engine->PostInit())
        {
            std::cerr << "Failed to post initialize the engine!" << std::endl;
            return false;
        }

        return true;
    }

    void EditorApp::Run()
    {
        float fixedTimer = 0;
        while (m_IsRunning && !m_Engine->GetContext()->WindowSys->ShouldClose())
        {
            m_Timer.Start();
            float dt        = m_Timer.GetDeltaTime();
            Time::DeltaTime = dt;
            Time::ElapsedTime += dt;

            // Fixed Tick
            fixedTimer += dt;
            while (fixedTimer >= Time::FixedDeltaTime)
            {
                m_GUISystem.OnFixedTick();
                fixedTimer -= Time::FixedDeltaTime;
            }

            // Tick Window System
            if (!m_Engine->GetContext()->WindowSys->Tick())
            {
                return;
            }

            // Dispatch Events
            m_Engine->GetContext()->EventSys->DispatchEvents();

            // GUI Begin
            m_GUISystem.Begin();

            // GUI Draw
            m_GUISystem.OnTick(dt);

            // GUI End
            m_GUISystem.End();

            m_Engine->GetContext()->RenderSys->Present();

            m_Timer.Stop();
        }

        Shutdown();
    }

    void EditorApp::Shutdown()
    {
        SNOW_LEOPARD_INFO("[Editor] Shutting Down...");

        // Save active project
        auto activeProject = g_EngineContext->ProjectMngr->GetActiveProject();
        if (activeProject != nullptr)
        {
            activeProject->Save();
        }

        // GUI Shutdown
        m_GUISystem.Shutdown();

        DesktopApp::Shutdown();
    }
} // namespace SnowLeopardEngine::Editor