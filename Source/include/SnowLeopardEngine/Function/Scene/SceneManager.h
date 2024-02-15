#pragma once

#include "SnowLeopardEngine/Core/Base/EngineSubSystem.h"
#include "SnowLeopardEngine/Function/Scene/LogicScene.h"

namespace SnowLeopardEngine
{
    class SceneManager final : public EngineSubSystem
    {
    public:
        DECLARE_SUBSYSTEM(SceneManager)

        Ref<LogicScene> CreateScene(const std::string& name);
    };
} // namespace SnowLeopardEngine
