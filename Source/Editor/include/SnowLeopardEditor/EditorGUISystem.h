#pragma once

namespace SnowLeopardEngine::Editor
{
    class EditorGUISystem
    {
    public:
        void Init();
        void Shutdown();

        void OnFixedTick();
        void OnTick(float deltaTime);

        void Begin();
        void End();

    private:
        void SetupUIStyle();
    };
} // namespace SnowLeopardEngine::Editor