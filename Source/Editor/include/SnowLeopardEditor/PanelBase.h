#pragma once

#include "SnowLeopardEngine/Core/Base/Base.h"

namespace SnowLeopardEngine::Editor
{
    struct PanelCommonInitInfo
    {
        std::string ProjectFilePath;
    };

    class PanelBase
    {
    public:
        PanelBase()          = default;
        virtual ~PanelBase() = default;

        virtual void Init(const PanelCommonInitInfo& initInfo) {}
        virtual void OnFixedTick() {}
        virtual void OnTick(float deltaTime) {}
        virtual void Shutdown() {}
    };
} // namespace SnowLeopardEngine::Editor