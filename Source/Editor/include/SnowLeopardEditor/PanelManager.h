#pragma once

#include "SnowLeopardEditor/PanelBase.h"
#include "SnowLeopardEngine/Core/Base/Base.h"

namespace SnowLeopardEngine::Editor
{
    class PanelManager
    {
    public:
        static void Init();
        static void OnFixedTick();
        static void OnTick(float deltaTime);
        static void Shutdown();

    private:
        static std::vector<Ref<PanelBase>> s_Panels;
    };
} // namespace SnowLeopardEngine::Editor