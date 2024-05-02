#pragma once

#include "SnowLeopardEngine/Core/Math/AABB.h"
#include "SnowLeopardEngine/Function/Rendering/FrameGraph/TransientResources.h"
#include "SnowLeopardEngine/Function/Rendering/FrameUniform.h"
#include "SnowLeopardEngine/Function/Rendering/LightProbe.h"
#include "SnowLeopardEngine/Function/Rendering/LightUniform.h"
#include "SnowLeopardEngine/Function/Rendering/RenderTypeDef.h"
#include "SnowLeopardEngine/Function/Rendering/Renderable.h"
#include "SnowLeopardEngine/Function/Rendering/Tools/CubemapConverter.h"
#include "SnowLeopardEngine/Function/Rendering/Tools/IBLDataGenerator.h"
#include "SnowLeopardEngine/Function/Rendering/WorldRenderer/Passes/BlitPass.h"
#include "SnowLeopardEngine/Function/Rendering/WorldRenderer/Passes/BloomPass.h"
#include "SnowLeopardEngine/Function/Rendering/WorldRenderer/Passes/DeferredLightingPass.h"
#include "SnowLeopardEngine/Function/Rendering/WorldRenderer/Passes/FXAAPass.h"
#include "SnowLeopardEngine/Function/Rendering/WorldRenderer/Passes/FinalPass.h"
#include "SnowLeopardEngine/Function/Rendering/WorldRenderer/Passes/GBufferPass.h"
#include "SnowLeopardEngine/Function/Rendering/WorldRenderer/Passes/GaussianBlurPass.h"
#include "SnowLeopardEngine/Function/Rendering/WorldRenderer/Passes/GrassPass.h"
#include "SnowLeopardEngine/Function/Rendering/WorldRenderer/Passes/InGameGUIPass.h"
#include "SnowLeopardEngine/Function/Rendering/WorldRenderer/Passes/SSAOPass.h"
#include "SnowLeopardEngine/Function/Rendering/WorldRenderer/Passes/ShadowPrePass.h"
#include "SnowLeopardEngine/Function/Rendering/WorldRenderer/Passes/SkyboxPass.h"
#include "SnowLeopardEngine/Function/Rendering/WorldRenderer/Passes/ToneMappingPass.h"
#include "SnowLeopardEngine/Function/Rendering/WorldRenderer/Passes/TransparencyComposePass.h"
#include "SnowLeopardEngine/Function/Rendering/WorldRenderer/Passes/WeightedBlendedPass.h"
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
        void UpdateLightUniform(const AABB& renderablesAABB);

        std::vector<Renderable> FilterVisableRenderables(std::span<Renderable> renderables,
                                                         const glm::mat4&      cameraViewProjection);

        std::vector<Renderable> FilterRenderables(std::span<Renderable> src, auto&& predicate);

        RenderableGroups FilterRenderableGroups(std::span<Renderable> renderables);

        AABB GetRenderablesAABB(std::span<Renderable> renderables);

    private:
        // -------- Context --------
        Scope<RenderContext> m_RenderContext = nullptr;
        Rect2D               m_Viewport;

        // -------- FrameGraph Transient Resource Manager --------
        Scope<TransientResources> m_TransientResources = nullptr;

        // -------- Tools --------
        Scope<CubemapConverter> m_CubemapConverter = nullptr;
        Scope<IBLDataGenerator> m_IBLDataGenerator = nullptr;

        // -------- Passes --------
        Scope<ShadowPrePass>           m_ShadowPrePass           = nullptr;
        Scope<GBufferPass>             m_GBufferPass             = nullptr;
        Scope<GrassPass>               m_GrassPass               = nullptr;
        Scope<WeightedBlendedPass>     m_WeightedBlendedPass     = nullptr;
        Scope<SSAOPass>                m_SSAOPass                = nullptr;
        Scope<GaussianBlurPass>        m_GaussianBlurPass        = nullptr;
        Scope<DeferredLightingPass>    m_DeferredLightingPass    = nullptr;
        Scope<SkyboxPass>              m_SkyboxPass              = nullptr;
        Scope<TransparencyComposePass> m_TransparencyComposePass = nullptr;
        Scope<BloomPass>               m_BloomPass               = nullptr;
        Scope<ToneMappingPass>         m_ToneMappingPass         = nullptr;
        Scope<FXAAPass>                m_FXAAPass                = nullptr;
        Scope<InGameGUIPass>           m_InGameGUIPass           = nullptr;
        Scope<BlitPass>              m_BlitPass              = nullptr;
        Scope<FinalPass>               m_FinalPass               = nullptr;

        // -------- Renderables --------
        std::vector<Renderable> m_Renderables;

        // -------- Camera, Lights, Skybox, etc. --------
        Entity              m_MainCamera;
        Entity              m_DirectionalLight;
        std::vector<Entity> m_PointLights;
        Texture             m_Skybox;

        // -------- IBL Data --------
        LightProbe m_GlobalLightProbe;
        Texture    m_BrdfLUT;

        // -------- UBO --------
        FrameUniform m_FrameUniform;
        LightUniform m_LightUniform;
    };
} // namespace SnowLeopardEngine