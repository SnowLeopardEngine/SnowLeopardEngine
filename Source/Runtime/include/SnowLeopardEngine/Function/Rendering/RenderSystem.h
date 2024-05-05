#pragma once

#include "SnowLeopardEngine/Core/Base/EngineSubSystem.h"
#include "SnowLeopardEngine/Core/Event/EventHandler.h"
#include "SnowLeopardEngine/Core/Event/SceneEvents.h"
#include "SnowLeopardEngine/Function/Rendering/GraphicsContext.h"
#include "SnowLeopardEngine/Function/Rendering/RenderTypeDef.h"
#include "SnowLeopardEngine/Function/Rendering/WorldRenderer/WorldRenderer.h"

namespace SnowLeopardEngine
{
    class FrameBuffer;

    class RenderSystem final : public EngineSubSystem
    {
    public:
        DECLARE_SUBSYSTEM(RenderSystem)

        void OnTick(float deltaTime);
        void Present();

        void SetRenderTarget(const Ref<FrameBuffer>& renderTarget) {}
        inline void UpdateViewport(const Rect2D& viewport) { m_Renderer.UpdateViewport(viewport); }

        Ref<GraphicsContext> GetGraphicsContext() const { return m_Context; }
        Ref<RenderContext>   GetGlobalRenderContext() const { return m_GlobalRenderContext; }

    private:
        void OnLogicSceneLoaded(const LogicSceneLoadedEvent& e);

    protected:
        Ref<GraphicsContext> m_Context             = nullptr;
        Ref<RenderContext>   m_GlobalRenderContext = nullptr;
        WorldRenderer        m_Renderer;

        EventHandler<LogicSceneLoadedEvent> m_LogicSceneLoadedHandler = [this](const LogicSceneLoadedEvent& e) {
            OnLogicSceneLoaded(e);
        };
    };
} // namespace SnowLeopardEngine