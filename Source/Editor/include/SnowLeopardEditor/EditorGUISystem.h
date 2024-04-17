#pragma once

#include "SnowLeopardEngine/Core/Base/Base.h"

namespace SnowLeopardEngine::Editor
{
    struct EditorGUISystemInitInfo
    {
        std::string ProjectFilePath;
    };

    class EditorGUISystem
    {
    public:
        void Init(const EditorGUISystemInitInfo& initInfo);
        void Shutdown();

        void OnFixedTick();
        void OnTick(float deltaTime);

        void Begin();
        void End();

    private:
        void SetupUIStyle();
    };
} // namespace SnowLeopardEngine::Editor