#pragma once

#include "SnowLeopardEditor/PanelBase.h"
#include "SnowLeopardEngine/Core/Base/Base.h"

namespace SnowLeopardEngine::Editor
{
    class PanelManager
    {
    public:
        static void Init();
        static void OnTick(float deltaTime);

    private:
        static std::vector<Ref<PanelBase>> s_Panels;
    };
} // namespace SnowLeopardEngine::Editor