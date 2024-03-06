#pragma once

#include "SnowLeopardEngine/Core/Base/EngineSubSystem.h"
#include "SnowLeopardEngine/Function/Rendering/GraphicsAPI.h"
#include "SnowLeopardEngine/Function/Rendering/GraphicsContext.h"
#include "SnowLeopardEngine/Function/Rendering/Pipeline/Pipeline.h"
#include "entt/entity/fwd.hpp"

namespace SnowLeopardEngine
{
    class RenderSystem final : public EngineSubSystem
    {
    public:
        DECLARE_SUBSYSTEM(RenderSystem)

        void OnTick(float deltaTime);
        void Present();

        const Ref<GraphicsAPI>& GetAPI() const { return m_API; }
        const Ref<Pipeline>&    GetPipeline() const { return m_Pipeline; }

    private:
        void Draw();

    protected:
        Ref<GraphicsContext> m_Context;
        Ref<GraphicsAPI>     m_API;
        Ref<Pipeline>        m_Pipeline;

        // TODO: Clean code
        bool                      m_LoadedScene = false;
        std::vector<entt::entity> m_GeometryGroup;
    };
} // namespace SnowLeopardEngine