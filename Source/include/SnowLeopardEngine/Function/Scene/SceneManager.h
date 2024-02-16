#pragma once

#include "SnowLeopardEngine/Core/Base/EngineSubSystem.h"
#include "SnowLeopardEngine/Function/Scene/LogicScene.h"

namespace SnowLeopardEngine
{
    class SceneManager final : public EngineSubSystem
    {
    public:
        DECLARE_SUBSYSTEM(SceneManager)

        Ref<LogicScene> CreateScene(const std::string& name, bool active = false);

        void SetActiveScene(const Ref<LogicScene>& activeScene);

        void OnLoad();
        void OnTick(float deltaTime);
        void OnFixedTick();
        void OnUnload();

    private:
        Ref<LogicScene> m_ActiveScene = nullptr;
    };
} // namespace SnowLeopardEngine
