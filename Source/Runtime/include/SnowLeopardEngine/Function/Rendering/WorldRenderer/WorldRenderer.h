#pragma once

#include "SnowLeopardEngine/Core/Math/AABB.h"
#include "SnowLeopardEngine/Function/Rendering/Converters/CubemapConverter.h"
#include "SnowLeopardEngine/Function/Rendering/FrameGraph/TransientResources.h"
#include "SnowLeopardEngine/Function/Rendering/FrameUniform.h"
#include "SnowLeopardEngine/Function/Rendering/LightUniform.h"
#include "SnowLeopardEngine/Function/Rendering/RenderTypeDef.h"
#include "SnowLeopardEngine/Function/Rendering/Renderable.h"
#include "SnowLeopardEngine/Function/Rendering/WorldRenderer/Passes/DeferredLightingPass.h"
#include "SnowLeopardEngine/Function/Rendering/WorldRenderer/Passes/FXAAPass.h"
#include "SnowLeopardEngine/Function/Rendering/WorldRenderer/Passes/FinalPass.h"
#include "SnowLeopardEngine/Function/Rendering/WorldRenderer/Passes/GBufferPass.h"
#include "SnowLeopardEngine/Function/Rendering/WorldRenderer/Passes/GaussianBlurPass.h"
#include "SnowLeopardEngine/Function/Rendering/WorldRenderer/Passes/SSAOPass.h"
#include "SnowLeopardEngine/Function/Rendering/WorldRenderer/Passes/ShadowPrePass.h"
#include "SnowLeopardEngine/Function/Rendering/WorldRenderer/Passes/SkyboxPass.h"
#include "SnowLeopardEngine/Function/Rendering/WorldRenderer/Passes/ToneMappingPass.h"
#include "SnowLeopardEngine/Function/Scene/Entity.h"

namespace SnowLeopardEngine
{
    class LogicScene;
    class Entity;

    class WorldRenderer
    {
    public:
        void Init();
        void Shutdown();

        void OnLogicSceneLoaded(LogicScene* scene);

        void RenderFrame(float deltaTime);

    private:
        void CreateTools();
        void CreatePasses();

        void CookRenderableScene();

        void UpdateFrameUniform();
        void UpdateLightUniform();

        std::vector<Renderable> FilterVisableRenderables(std::span<Renderable> renderables,
                                                         const glm::mat4&      cameraViewProjection);

        RenderableGroups FilterRenderableGroups(std::span<Renderable> renderables);

        AABB GetVisableAABB(std::span<Renderable> visableRenderables);
        AABB GetRenderableSceneAABB();

    private:
        // -------- Context --------
        Scope<RenderContext> m_RenderContext = nullptr;
        Rect2D               m_Viewport;

        // -------- FrameGraph Transient Resource Manager --------
        Scope<TransientResources> m_TransientResources = nullptr;

        // -------- Tools --------
        Scope<CubemapConverter> m_CubemapConverter = nullptr;

        // -------- Passes --------
        Scope<ShadowPrePass>        m_ShadowPrePass        = nullptr;
        Scope<GBufferPass>          m_GBufferPass          = nullptr;
        Scope<SSAOPass>             m_SSAOPass             = nullptr;
        Scope<GaussianBlurPass>     m_GaussianBlurPass     = nullptr;
        Scope<DeferredLightingPass> m_DeferredLightingPass = nullptr;
        Scope<SkyboxPass>           m_SkyboxPass           = nullptr;
        Scope<ToneMappingPass>      m_ToneMappingPass      = nullptr;
        Scope<FXAAPass>             m_FXAAPass             = nullptr;
        Scope<FinalPass>            m_FinalPass            = nullptr;

        // -------- Renderables --------
        std::vector<Renderable> m_Renderables;

        // -------- Camera, Lights, Skybox --------
        Entity              m_MainCamera;
        Entity              m_DirectionalLight;
        std::vector<Entity> m_PointLights;
        Texture             m_Skybox;

        // -------- UBO --------
        FrameUniform m_FrameUniform;
        LightUniform m_LightUniform;
    };
} // namespace SnowLeopardEngine