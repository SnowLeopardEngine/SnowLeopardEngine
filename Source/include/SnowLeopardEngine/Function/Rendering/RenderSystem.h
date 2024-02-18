#pragma once

#include "SnowLeopardEngine/Core/Base/EngineSubSystem.h"
#include "SnowLeopardEngine/Function/Rendering/GraphicsContext.h"

namespace SnowLeopardEngine
{
    class RenderSystem final : public EngineSubSystem
    {
    public:
        DECLARE_SUBSYSTEM(RenderSystem)

        void OnTick(float deltaTime);

        void Present();

    protected:
        Ref<GraphicsContext> m_Context;
    };
} // namespace SnowLeopardEngine