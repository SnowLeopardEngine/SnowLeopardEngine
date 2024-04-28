#pragma once

#include "SnowLeopardEngine/Function/Rendering/FrameGraph/TransientResources.h"
#include "SnowLeopardEngine/Function/Rendering/FrameUniform.h"
#include "SnowLeopardEngine/Function/Rendering/Renderable.h"
#include "SnowLeopardEngine/Function/Rendering/WorldRenderer/Passes/FinalPass.h"
#include "SnowLeopardEngine/Function/Rendering/WorldRenderer/Passes/GBufferPass.h"
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

    private:
        // -------- Context --------
        Scope<RenderContext>      m_RenderContext      = nullptr;
        Scope<TransientResources> m_TransientResources = nullptr;

        // -------- Passes --------
        Scope<GBufferPass> m_GBufferPass = nullptr;
        Scope<FinalPass>   m_FinalPass   = nullptr;

        // -------- Renderables --------

        std::vector<Renderable> m_Renderables;

        // -------- Frame UBO --------
        Entity       m_MainCamera;
        Entity       m_DirectionalLight;
        FrameUniform m_FrameUniform;
    };
} // namespace SnowLeopardEngine