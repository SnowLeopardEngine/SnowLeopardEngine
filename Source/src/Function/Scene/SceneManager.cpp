#include "SnowLeopardEngine/Function/Scene/SceneManager.h"

namespace SnowLeopardEngine
{
    Ref<LogicScene> SceneManager::CreateScene(const std::string& name)
    {
        return CreateRef<LogicScene>(name);
    }
} // namespace SnowLeopardEngine
