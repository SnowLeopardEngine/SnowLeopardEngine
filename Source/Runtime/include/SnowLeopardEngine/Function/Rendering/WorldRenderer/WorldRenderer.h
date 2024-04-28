#pragma once

#include "SnowLeopardEngine/Core/Math/AABB.h"
#include "SnowLeopardEngine/Function/Rendering/FrameGraph/TransientResources.h"
#include "SnowLeopardEngine/Function/Rendering/FrameUniform.h"
#include "SnowLeopardEngine/Function/Rendering/Renderable.h"
#include "SnowLeopardEngine/Function/Rendering/WorldRenderer/Passes/DeferredLightingPass.h"
#include "SnowLeopardEngine/Function/Rendering/WorldRenderer/Passes/FinalPass.h"
#include "SnowLeopardEngine/Function/Rendering/WorldRenderer/Passes/GBufferPass.h"
#include "SnowLeopardEngine/Function/Rendering/WorldRenderer/Passes/ShadowPrePass.h"
#include "SnowLeopardEngine/Function/Scene/Entity.h"

namespace SnowLeopardEngine
{
    class LogicScene;
    class Entity;

    class WorldRenderer
    {
    public:
        void Init();

        void OnLogicSceneLoaded(LogicScene* scene);

        void RenderFrame(float deltaTime);

    private:
        void CreatePasses();

        AABB GetRenderableSceneAABB();

    private:
        // -------- Context --------
        Scope<RenderContext>      m_RenderContext      = nullptr;
        Scope<TransientResources> m_TransientResources = nullptr;

        // -------- Passes --------
        Scope<ShadowPrePass>        m_ShadowPrePass        = nullptr;
        Scope<GBufferPass>          m_GBufferPass          = nullptr;
        Scope<DeferredLightingPass> m_DeferredLightingPass = nullptr;
        Scope<FinalPass>            m_FinalPass            = nullptr;

        // -------- Renderables --------

        std::vector<Renderable> m_Renderables;

        // -------- Frame UBO --------
        Entity       m_MainCamera;
        Entity       m_DirectionalLight;
        FrameUniform m_FrameUniform;
    };
} // namespace SnowLeopardEngine