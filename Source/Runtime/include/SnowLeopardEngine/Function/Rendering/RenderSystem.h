#pragma once

#include "SnowLeopardEngine/Core/Base/EngineSubSystem.h"
#include "SnowLeopardEngine/Function/Rendering/GraphicsAPI.h"
#include "SnowLeopardEngine/Function/Rendering/GraphicsContext.h"
#include "entt/entity/fwd.hpp"

namespace SnowLeopardEngine
{
    class FrameBuffer;

    class RenderSystem final : public EngineSubSystem
    {
    public:
        DECLARE_SUBSYSTEM(RenderSystem)

        void OnTick(float deltaTime);
        void Present();

        void SetRenderTarget(const Ref<FrameBuffer>& renderTarget) { m_RenderTarget = renderTarget; }

        const Ref<GraphicsAPI>& GetAPI() const { return m_API; }

    private:
        void Draw();

    protected:
        Ref<GraphicsContext> m_Context;
        Ref<GraphicsAPI>     m_API;

        // TODO: Clean code
        bool                      m_LoadedScene = false;
        std::vector<entt::entity> m_ShadowGroup;
        std::vector<entt::entity> m_GeometryGroup;
        std::vector<entt::entity> m_SkyGroup;

        Ref<FrameBuffer> m_RenderTarget;
    };
} // namespace SnowLeopardEngine