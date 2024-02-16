#include "SnowLeopardEngine/Function/Scene/SceneManager.h"
#include "SnowLeopardEngine/Core/Log/Log.h"

namespace SnowLeopardEngine
{
    SceneManager::SceneManager()
    {
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

    void SceneManager::SetActiveScene(const Ref<LogicScene>& activeScene) { m_ActiveScene = activeScene; }

    void SceneManager::OnLoad()
    {
        if (m_ActiveScene)
        {
            m_ActiveScene->OnLoad();
        }
    }

    void SceneManager::OnTick(float deltaTime)
    {
        if (m_ActiveScene)
        {
            m_ActiveScene->OnTick(deltaTime);
        }
    }

    void SceneManager::OnFixedTick()
    {
        if (m_ActiveScene)
        {
            m_ActiveScene->OnFixedTick();
        }
    }

    void SceneManager::OnUnload()
    {
        if (m_ActiveScene)
        {
            m_ActiveScene->OnUnload();
        }
    }
} // namespace SnowLeopardEngine
