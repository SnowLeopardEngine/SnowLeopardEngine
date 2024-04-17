#include "SnowLeopardEngine/Function/Project/ProjectManager.h"
#include "SnowLeopardEngine/Engine/EngineContext.h"

namespace SnowLeopardEngine
{
    ProjectManager::ProjectManager()
    {
        SNOW_LEOPARD_CORE_INFO("[ProjectManager] Initialized");
        m_State = SystemState::InitOk;
    }

    ProjectManager::~ProjectManager()
    {
        SNOW_LEOPARD_CORE_INFO("[ProjectManager] Shutting Down...");
        m_State = SystemState::ShutdownOk;
    }
} // namespace SnowLeopardEngine