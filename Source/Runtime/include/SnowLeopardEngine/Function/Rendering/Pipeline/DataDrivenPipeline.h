#pragma once

#include "SnowLeopardEngine/Function/Rendering/DzMaterial/DzMaterial.h"
#include "SnowLeopardEngine/Function/Rendering/DzShader/DzShaderTypeDef.h"
#include "SnowLeopardEngine/Function/Rendering/Instancing.h"
#include "SnowLeopardEngine/Function/Rendering/Pipeline/PipelineState.h"
#include "SnowLeopardEngine/Function/Rendering/RHI/FrameBuffer.h"
#include "SnowLeopardEngine/Function/Rendering/RenderTypeDef.h"
#include "SnowLeopardEngine/Function/Rendering/SceneUniform.h"

namespace SnowLeopardEngine
{
    class LogicScene;
    class Shader;

    class DataDrivenPipeline
    {
    public:
        inline void SetRenderTarget(const Ref<FrameBuffer>& renderTarget) { m_RenderTarget = renderTarget; }
        void        SetupPipeline(LogicScene* scene);
        void        RenderScene();

    private:
        void GroupAndCompileShaders();
        void HandleInstancingBatches();

        void UpdateSceneUniform();
        void RenderShadow();
        void RenderInstancedGeometry(const InstancingBatchGroup& group);
        void RenderGeometry(entt::entity geometry);
        void RenderSky(entt::entity sky);
        void RenderUI(entt::entity ui);
        void RenderMeshItem(entt::entity    geometry,
                            entt::registry& registry,
                            MeshItem&       meshItem,
                            const DzPass&   dzPass,
                            DzMaterial&     material,
                            int&            resourceBinding) const;

        void RenderMeshItemInstanced(const std::vector<entt::entity>& groupEntities,
                                     entt::registry&                  registry,
                                     MeshItem&                        meshItem,
                                     const DzShader&                  dzShader,
                                     const DzPass&                    dzPass,
                                     DzMaterial&                      material,
                                     int&                             resourceBinding) const;

        static void ProcessPipelineState(const std::string& pipelineStateName,
                                         const std::string& pipelineStateValue,
                                         PipelineState&     pipelineState);

        static void SetMaterialProperties(const std::string& propertyName,
                                          const std::string& shaderPropertyName,
                                          const std::string& propertyType,
                                          DzMaterial&        material,
                                          const Ref<Shader>& shader,
                                          int&               resourceBinding);

        std::vector<InstancingBatchGroup> m_InstancingBatchGroups;

        entt::entity m_MainCamera;
        entt::entity m_DirectionalLight;
        SceneUniform m_SceneUniform;

        std::vector<entt::entity> m_ShadowGroup;
        std::vector<entt::entity> m_GeometryGroup;
        std::vector<entt::entity> m_SkyGroup;
        std::vector<entt::entity> m_UIGroup;

        Ref<FrameBuffer> m_RenderTarget = nullptr;
        LogicScene*      m_Scene        = nullptr;
    };
} // namespace SnowLeopardEngine