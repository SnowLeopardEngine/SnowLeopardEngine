#include "SnowLeopardEngine/Function/Scene/SceneManager.h"
#include "SnowLeopardEngine/Engine/EngineContext.h"
#include "SnowLeopardEngine/Function/Scene/Entity.h"

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

        // Add default entities

        // Default directional light
        Entity directionalLight = scene->CreateEntity("Directional Light");
        directionalLight.AddComponent<DirectionalLightComponent>();

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

            // Cook PhysicsScene!
            g_EngineContext->PhysicsSys->CookPhysicsScene(m_ActiveScene);
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
