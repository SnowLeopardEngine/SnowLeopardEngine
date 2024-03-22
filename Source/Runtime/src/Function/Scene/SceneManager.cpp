#include "SnowLeopardEngine/Function/Scene/SceneManager.h"
#include "SnowLeopardEngine/Core/Profiling/Profiling.h"
#include "SnowLeopardEngine/Engine/EngineContext.h"
#include "SnowLeopardEngine/Function/Scene/Entity.h"
#include "SnowLeopardEngine/Function/Scene/LayerManager.h"

namespace SnowLeopardEngine
{
    SceneManager::SceneManager()
    {
        // TODO: When project loading & saving, load the project specific layers & tags.
        LayerMaskManager::Init();

        SNOW_LEOPARD_CORE_INFO("[SceneManager] Initialized");
        m_State = SystemState::InitOk;
    }

    SceneManager::~SceneManager()
    {
        SNOW_LEOPARD_CORE_INFO("[SceneManager] Shutting Down...");
        m_State = SystemState::ShutdownOk;
    }

    Ref<LogicScene> SceneManager::CreateScene(const std::string& name, bool active)
    {
        auto scene = CreateRef<LogicScene>(name);

        if (active)
        {
            m_ActiveScene = scene;
        }
        return scene;
    }

    void SceneManager::SetActiveScene(const Ref<LogicScene>& activeScene)
    {
        if (m_ActiveScene == activeScene)
        {
            return;
        }

        if (m_ActiveScene != nullptr)
        {
            m_ActiveScene->OnUnload();
        }

        if (activeScene != nullptr)
        {
            activeScene->OnLoad();
            m_ActiveScene = activeScene;
        }
    }

    void SceneManager::OnLoad()
    {
        SNOW_LEOPARD_PROFILE_FUNCTION
        if (m_ActiveScene)
        {
            m_ActiveScene->OnLoad();
        }
    }

    void SceneManager::OnTick(float deltaTime)
    {
        SNOW_LEOPARD_PROFILE_FUNCTION
        if (m_ActiveScene)
        {
            m_ActiveScene->OnTick(deltaTime);
        }
    }

    void SceneManager::OnFixedTick()
    {
        SNOW_LEOPARD_PROFILE_FUNCTION
        if (m_ActiveScene)
        {
            m_ActiveScene->OnFixedTick();
        }
    }

    void SceneManager::OnUnload()
    {
        SNOW_LEOPARD_PROFILE_FUNCTION
        if (m_ActiveScene)
        {
            m_ActiveScene->OnUnload();
        }
    }
} // namespace SnowLeopardEngine
