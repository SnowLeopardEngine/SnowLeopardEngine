#include "SnowLeopardEngine/Function/Rendering/Pipeline/DataDrivenPipeline.h"
#include "SnowLeopardEngine/Core/Base/Macro.h"
#include "SnowLeopardEngine/Core/Profiling/Profiling.h"
#include "SnowLeopardEngine/Core/Time/Time.h"
#include "SnowLeopardEngine/Engine/EngineContext.h"
#include "SnowLeopardEngine/Function/Rendering/DzMaterial/DzMaterial.h"
#include "SnowLeopardEngine/Function/Rendering/DzShader/DzShaderManager.h"
#include "SnowLeopardEngine/Function/Rendering/DzShader/DzShaderTypeDef.h"
#include "SnowLeopardEngine/Function/Rendering/GraphicsAPI.h"
#include "SnowLeopardEngine/Function/Rendering/RHI/Shader.h"
#include "SnowLeopardEngine/Function/Scene/Components.h"
#include "SnowLeopardEngine/Function/Scene/LogicScene.h"

#include "entt/entity/entity.hpp"
#include <magic_enum.hpp>

const uint32_t kMaxInstanceBatchGroup = 100;

namespace SnowLeopardEngine
{
    void DataDrivenPipeline::SetupPipeline(LogicScene* scene)
    {
        SNOW_LEOPARD_PROFILE_FUNCTION

        m_Scene = scene;
        GroupAndCompileShaders();
        HandleInstancingBatches();
    }

    void DataDrivenPipeline::RenderScene()
    {
        SNOW_LEOPARD_PROFILE_FUNCTION

        if (m_Scene == nullptr)
        {
            return;
        }

        auto& registry = m_Scene->GetRegistry();

        // Find main camera & directional light
        m_MainCamera = registry.view<TransformComponent, CameraComponent>().front();
        if (m_MainCamera == entt::null)
        {
            return;
        }
        m_DirectionalLight = registry.view<TransformComponent, DirectionalLightComponent>().front();
        if (m_DirectionalLight == entt::null)
        {
            return;
        }

        // Setup camera properties & clear color
        auto& mainCameraComponent          = registry.get<CameraComponent>(m_MainCamera);
        mainCameraComponent.ViewportWidth  = g_EngineContext->WindowSys->GetWidth();
        mainCameraComponent.ViewportHeight = g_EngineContext->WindowSys->GetHeight();
        if (mainCameraComponent.ClearFlags == CameraClearFlags::Color)
        {
            g_EngineContext->RenderSys->GetAPI()->ClearColor(mainCameraComponent.ClearColor, ClearBit::Default);
        }
        else
        {
            g_EngineContext->RenderSys->GetAPI()->ClearColor(glm::vec4(0, 0, 0, 1), ClearBit::Default);
        }

        // Update scene uniforms
        UpdateSceneUniform();

        // Render shadow
        RenderShadow();

        // If there is customized RT
        if (m_RenderTarget != nullptr)
        {
            m_RenderTarget->Bind();

            auto     desc   = m_RenderTarget->GetDesc();
            uint32_t width  = desc.Width;
            uint32_t height = desc.Height;

            g_EngineContext->RenderSys->GetAPI()->UpdateViewport(0, 0, width, height);
            mainCameraComponent.ViewportWidth  = width;
            mainCameraComponent.ViewportHeight = height;

            // Update scene uniforms
            UpdateSceneUniform();
        }

        if (mainCameraComponent.ClearFlags == CameraClearFlags::Color)
        {
            g_EngineContext->RenderSys->GetAPI()->ClearColor(mainCameraComponent.ClearColor, ClearBit::Default);
        }
        else
        {
            g_EngineContext->RenderSys->GetAPI()->ClearColor(glm::vec4(0, 0, 0, 1), ClearBit::Default);
        }

        // Render instanced geometry
        for (auto& group : m_InstancingBatchGroups)
        {
            SNOW_LEOPARD_PROFILE_NAMED_SCOPE("Render Instanced Geometry")

            RenderInstancedGeometry(group);
        }

        // Render geometry
        for (auto& geometry : m_GeometryGroup)
        {
            SNOW_LEOPARD_PROFILE_NAMED_SCOPE("Render Geometry")

            RenderGeometry(geometry);
        }

        // Render skyboxes
        for (auto& sky : m_SkyGroup)
        {
            SNOW_LEOPARD_PROFILE_NAMED_SCOPE("Render Sky")

            RenderSky(sky);
        }

        // TODO: Render other elements

        if (m_RenderTarget != nullptr)
        {
            m_RenderTarget->Unbind();
        }
    }

    void DataDrivenPipeline::GroupAndCompileShaders()
    {
        SNOW_LEOPARD_PROFILE_FUNCTION

        SNOW_LEOPARD_CORE_ASSERT(m_Scene != nullptr, "[DataDrivenPipeline] Assertion failed, m_Scene == nullptr");

        m_ShadowGroup.clear();
        m_GeometryGroup.clear();
        m_SkyGroup.clear();

        auto& registry = m_Scene->GetRegistry();

        registry.view<TransformComponent, CameraComponent>().each(
            [this](entt::entity entity, TransformComponent& transform, CameraComponent& camera) {
                if (camera.ClearFlags != CameraClearFlags::Skybox || camera.SkyboxMaterial == nullptr)
                {
                    return;
                }

                DzShaderManager::AddShader(camera.SkyboxMaterial->GetShaderName(),
                                           camera.SkyboxMaterial->GetShaderPath());

                auto queue = camera.SkyboxMaterial->GetTag("RenderQueue");
                if (queue == "Sky")
                {
                    m_SkyGroup.emplace_back(entity);
                }
            });

        registry.view<TransformComponent, DirectionalLightComponent>().each(
            [this](entt::entity entity, TransformComponent& transform, DirectionalLightComponent& directionalLight) {
                if (directionalLight.ShadowMaterial == nullptr)
                {
                    return;
                }

                DzShaderManager::AddShader(directionalLight.ShadowMaterial->GetShaderName(),
                                           directionalLight.ShadowMaterial->GetShaderPath());

                auto queue = directionalLight.ShadowMaterial->GetTag("RenderQueue");
                if (queue == "Shadow")
                {
                    m_ShadowGroup.emplace_back(entity);
                }
            });

        registry.view<TransformComponent, TerrainComponent, TerrainRendererComponent>().each(
            [this](entt::entity              entity,
                   TransformComponent&       transform,
                   TerrainComponent&         terrain,
                   TerrainRendererComponent& terrainRenderer) {
                if (terrainRenderer.Material == nullptr)
                {
                    return;
                }

                DzShaderManager::AddShader(terrainRenderer.Material->GetShaderName(),
                                           terrainRenderer.Material->GetShaderPath());

                auto queue = terrainRenderer.Material->GetTag("RenderQueue");
                if (queue == "Geometry")
                {
                    m_GeometryGroup.emplace_back(entity);
                }
            });

        registry.view<TransformComponent, MeshFilterComponent, MeshRendererComponent>().each(
            [this](entt::entity           entity,
                   TransformComponent&    transform,
                   MeshFilterComponent&   meshFilter,
                   MeshRendererComponent& meshRenderer) {
                if (meshRenderer.Material == nullptr)
                {
                    return;
                }

                DzShaderManager::AddShader(meshRenderer.Material->GetShaderName(),
                                           meshRenderer.Material->GetShaderPath());

                auto queue = meshRenderer.Material->GetTag("RenderQueue");
                if (queue == "Geometry")
                {
                    m_GeometryGroup.emplace_back(entity);
                }
            });

        DzShaderManager::Compile();
    }

    void DataDrivenPipeline::HandleInstancingBatches()
    {
        SNOW_LEOPARD_PROFILE_FUNCTION

        SNOW_LEOPARD_CORE_ASSERT(m_Scene != nullptr, "[DataDrivenPipeline] Assertion failed, m_Scene == nullptr");

        auto& registry = m_Scene->GetRegistry();

        // Arrange inner groups in geometry group (instancing + non-instancing)
        std::unordered_map<std::string, uint32_t> instancingShaderName2BatchIDMap;
        uint32_t                                  instancingBatchID = 0;
        m_InstancingBatchGroups.resize(kMaxInstanceBatchGroup);
        for (auto& geometry : m_GeometryGroup)
        {
            auto&                  transform = registry.get<TransformComponent>(geometry);
            std::vector<MeshItem*> meshItemPtrs;
            MeshRendererComponent  renderer;

            if (registry.any_of<MeshFilterComponent, MeshRendererComponent>(geometry))
            {
                auto& meshFilter = registry.get<MeshFilterComponent>(geometry);
                renderer         = registry.get<MeshRendererComponent>(geometry);
                for (auto& meshItem : meshFilter.Meshes->Items)
                {
                    meshItemPtrs.emplace_back(&meshItem);
                }
            }

            // Empty mesh and Submeshes are invalid here.
            if (meshItemPtrs.size() > 1 || meshItemPtrs.empty())
            {
                continue;
            }

            if (renderer.EnableInstancing)
            {
                auto dzShaderName = renderer.Material->GetShaderName();
                if (instancingShaderName2BatchIDMap.count(dzShaderName) == 0)
                {
                    m_InstancingBatchGroups[instancingBatchID].Entities.emplace_back(geometry);
                    m_InstancingBatchGroups[instancingBatchID].ShaderName   = dzShaderName;
                    m_InstancingBatchGroups[instancingBatchID].MeshItemName = meshItemPtrs[0]->Name;
                    instancingShaderName2BatchIDMap[dzShaderName]           = instancingBatchID;
                    instancingBatchID++;
                }
                else
                {
                    // The mesh item must be the same (same name)
                    uint32_t currentBatchID      = instancingShaderName2BatchIDMap[dzShaderName];
                    auto     currentMeshItemName = m_InstancingBatchGroups[currentBatchID].MeshItemName;
                    if (currentMeshItemName == meshItemPtrs[0]->Name)
                    {
                        m_InstancingBatchGroups[currentBatchID].Entities.emplace_back(geometry);
                    }
                }
            }
        }
    }

    void DataDrivenPipeline::UpdateSceneUniform()
    {
        SNOW_LEOPARD_PROFILE_FUNCTION

        SNOW_LEOPARD_CORE_ASSERT(m_Scene != nullptr, "[DataDrivenPipeline] Assertion failed, m_Scene == nullptr");

        auto& registry = m_Scene->GetRegistry();

        // Camera uniforms
        auto& mainCameraTransform       = registry.get<TransformComponent>(m_MainCamera);
        auto& mainCameraComponent       = registry.get<CameraComponent>(m_MainCamera);
        m_SceneUniform.ViewPos          = mainCameraTransform.Position;
        m_SceneUniform.ViewMatrix       = g_EngineContext->CameraSys->GetViewMatrix(mainCameraTransform);
        m_SceneUniform.ProjectionMatrix = g_EngineContext->CameraSys->GetProjectionMatrix(mainCameraComponent);

        // Light uniforms
        auto& directionalLightComponent          = registry.get<DirectionalLightComponent>(m_DirectionalLight);
        m_SceneUniform.DirectionalLightColor     = directionalLightComponent.Color;
        m_SceneUniform.DirectionalLightIntensity = directionalLightComponent.Intensity;
        m_SceneUniform.DirectionalLightDirection = directionalLightComponent.Direction;
        // TODO: View frustum get AABB and set borders.
        glm::mat4 lightProjection = glm::ortho(-150.0f, 150.0f, -150.0f, 150.0f, 1.0f, 10000.0f);
        auto      lightPos  = -1000.0f * directionalLightComponent.Direction; // simulate directional light position
        glm::mat4 lightView = glm::lookAt(lightPos, glm::vec3(0, 0, 0), glm::vec3(0.0f, 1.0f, 0.0f));
        m_SceneUniform.LightSpaceMatrix = lightProjection * lightView;

        // Misc uniforms
        m_SceneUniform.ElapsedTime = Time::ElapsedTime;
    }

    void DataDrivenPipeline::RenderShadow()
    {
        SNOW_LEOPARD_PROFILE_FUNCTION

        SNOW_LEOPARD_CORE_ASSERT(m_Scene != nullptr, "[DataDrivenPipeline] Assertion failed, m_Scene == nullptr");

        auto& registry = m_Scene->GetRegistry();

        auto& directionalLight = registry.get<DirectionalLightComponent>(m_DirectionalLight);
        if (directionalLight.ShadowMaterial == nullptr)
        {
            return;
        }

        PipelineState pipelineState;

        auto  shadowShaderName = directionalLight.ShadowMaterial->GetShaderName();
        auto& shadowShader     = DzShaderManager::GetShader(shadowShaderName);

        for (const auto& [pipelineStateName, pipelineStateValue] : shadowShader.PipelineStates)
        {
            ProcessPipelineState(pipelineStateName, pipelineStateValue, pipelineState);
        }

        for (auto& shadowPass : shadowShader.Passes)
        {

            for (const auto& [pipelineStateName, pipelineStateValue] : shadowPass.PipelineStates)
            {
                ProcessPipelineState(pipelineStateName, pipelineStateValue, pipelineState);
            }

            g_EngineContext->RenderSys->GetAPI()->SetPipelineState(pipelineState);
            DzShaderManager::BindPassResources(shadowPass);
            auto shader = DzShaderManager::GetCompiledPassShader(shadowPass.Name);

            for (auto& geometry : m_GeometryGroup)
            {
                Ref<DzMaterial> material = nullptr;

                // If it doesn't cast shadow / instancing is enabled, continue
                if (registry.any_of<MeshRendererComponent>(geometry))
                {
                    auto& meshRenderer = registry.get<MeshRendererComponent>(geometry);
                    if (!meshRenderer.CastShadow || meshRenderer.EnableInstancing)
                    {
                        continue;
                    }
                    material = meshRenderer.Material;
                }
                if (registry.any_of<TerrainRendererComponent>(geometry))
                {
                    auto& terrainRenderer = registry.get<TerrainRendererComponent>(geometry);
                    if (!terrainRenderer.CastShadow)
                    {
                        continue;
                    }
                    material = terrainRenderer.Material;
                }

                std::vector<MeshItem*> meshItemPtrs;

                if (registry.any_of<MeshFilterComponent>(geometry))
                {
                    auto& meshFilter = registry.get<MeshFilterComponent>(geometry);
                    for (auto& meshItem : meshFilter.Meshes->Items)
                    {
                        meshItemPtrs.emplace_back(&meshItem);
                    }
                }

                if (registry.any_of<TerrainComponent>(geometry))
                {
                    auto& terrain = registry.get<TerrainComponent>(geometry);
                    meshItemPtrs.emplace_back(&terrain.Mesh);
                }

                for (auto& meshItemPtr : meshItemPtrs)
                {
                    int resourceBinding = 0;
                    RenderMeshItem(geometry, registry, *meshItemPtr, shadowPass, *material, resourceBinding);
                }
            }

            DzShaderManager::UnbindPassResources(shadowPass);
        }

        // Restore viewport size after shadow mapping
        g_EngineContext->RenderSys->GetAPI()->UpdateViewport(
            0, 0, g_EngineContext->WindowSys->GetWidth(), g_EngineContext->WindowSys->GetHeight());
        auto& mainCamera          = registry.get<CameraComponent>(m_MainCamera);
        mainCamera.ViewportWidth  = g_EngineContext->WindowSys->GetWidth();
        mainCamera.ViewportHeight = g_EngineContext->WindowSys->GetHeight();
    }

    void DataDrivenPipeline::RenderInstancedGeometry(const InstancingBatchGroup& group)
    {
        SNOW_LEOPARD_PROFILE_FUNCTION

        SNOW_LEOPARD_CORE_ASSERT(m_Scene != nullptr, "[DataDrivenPipeline] Assertion failed, m_Scene == nullptr");

        auto& registry = m_Scene->GetRegistry();

        if (group.Entities.empty())
        {
            return;
        }

        auto  batchGeometry  = group.Entities[0];
        auto& batchTransform = registry.get<TransformComponent>(batchGeometry);
        auto  dzShaderName   = group.ShaderName;
        auto& dzShader       = DzShaderManager::GetShader(dzShaderName);

        std::vector<MeshItem*> meshItemPtrs;
        BaseRendererComponent  renderer;

        if (registry.any_of<MeshFilterComponent, MeshRendererComponent>(batchGeometry))
        {
            auto& meshFilter = registry.get<MeshFilterComponent>(batchGeometry);
            renderer         = registry.get<MeshRendererComponent>(batchGeometry);
            for (auto& meshItem : meshFilter.Meshes->Items)
            {
                meshItemPtrs.emplace_back(&meshItem);
            }
        }

        if (meshItemPtrs.size() > 1 || meshItemPtrs.empty())
        {
            // ERROR
            return;
        }

        PipelineState pipelineState;

        for (const auto& [pipelineStateName, pipelineStateValue] : dzShader.PipelineStates)
        {
            ProcessPipelineState(pipelineStateName, pipelineStateValue, pipelineState);
        }

        for (const auto& dzPass : dzShader.Passes)
        {
            for (const auto& [pipelineStateName, pipelineStateValue] : dzPass.PipelineStates)
            {
                ProcessPipelineState(pipelineStateName, pipelineStateValue, pipelineState);
            }

            g_EngineContext->RenderSys->GetAPI()->SetPipelineState(pipelineState);
            DzShaderManager::BindPassResources(dzPass);

            auto& meshItemPtr     = meshItemPtrs[0];
            int   resourceBinding = 0;
            RenderMeshItemInstanced(
                group.Entities, registry, *meshItemPtr, dzShader, dzPass, *renderer.Material, resourceBinding);
            DzShaderManager::UnbindPassResources(dzPass);
        }
    }

    void DataDrivenPipeline::RenderGeometry(entt::entity geometry)
    {
        SNOW_LEOPARD_PROFILE_FUNCTION

        SNOW_LEOPARD_CORE_ASSERT(m_Scene != nullptr, "[DataDrivenPipeline] Assertion failed, m_Scene == nullptr");

        auto& registry = m_Scene->GetRegistry();

        std::vector<MeshItem*> meshItemPtrs;
        BaseRendererComponent  renderer;

        if (registry.any_of<MeshFilterComponent, MeshRendererComponent>(geometry))
        {
            auto& meshFilter   = registry.get<MeshFilterComponent>(geometry);
            auto& meshRenderer = registry.get<MeshRendererComponent>(geometry);
            if (meshRenderer.EnableInstancing)
            {
                return;
            }
            renderer = meshRenderer;
            for (auto& meshItem : meshFilter.Meshes->Items)
            {
                meshItemPtrs.emplace_back(&meshItem);
            }
        }

        if (registry.any_of<TerrainComponent, TerrainRendererComponent>(geometry))
        {
            auto& terrain = registry.get<TerrainComponent>(geometry);
            renderer      = registry.get<TerrainRendererComponent>(geometry);
            meshItemPtrs.emplace_back(&terrain.Mesh);
        }

        auto  dzShaderName = renderer.Material->GetShaderName();
        auto& dzShader     = DzShaderManager::GetShader(dzShaderName);

        PipelineState pipelineState;

        for (const auto& [pipelineStateName, pipelineStateValue] : dzShader.PipelineStates)
        {
            ProcessPipelineState(pipelineStateName, pipelineStateValue, pipelineState);
        }

        for (const auto& dzPass : dzShader.Passes)
        {
            for (const auto& [pipelineStateName, pipelineStateValue] : dzPass.PipelineStates)
            {
                ProcessPipelineState(pipelineStateName, pipelineStateValue, pipelineState);
            }

            g_EngineContext->RenderSys->GetAPI()->SetPipelineState(pipelineState);
            DzShaderManager::BindPassResources(dzPass);

            for (auto& meshItemPtr : meshItemPtrs)
            {
                int resourceBinding = 0;
                RenderMeshItem(geometry, registry, *meshItemPtr, dzPass, *renderer.Material, resourceBinding);
            }

            DzShaderManager::UnbindPassResources(dzPass);
        }
    }

    void DataDrivenPipeline::RenderSky(entt::entity sky)
    {
        SNOW_LEOPARD_PROFILE_FUNCTION

        SNOW_LEOPARD_CORE_ASSERT(m_Scene != nullptr, "[DataDrivenPipeline] Assertion failed, m_Scene == nullptr");

        auto& registry = m_Scene->GetRegistry();

        auto& transform = registry.get<TransformComponent>(sky);
        auto& camera    = registry.get<CameraComponent>(sky);

        if (!camera.IsPrimary || camera.SkyboxMaterial == nullptr)
        {
            return;
        }

        auto  dzShaderName = camera.SkyboxMaterial->GetShaderName();
        auto& dzShader     = DzShaderManager::GetShader(dzShaderName);

        PipelineState pipelineState;

        for (const auto& [pipelineStateName, pipelineStateValue] : dzShader.PipelineStates)
        {
            ProcessPipelineState(pipelineStateName, pipelineStateValue, pipelineState);
        }

        for (const auto& dzPass : dzShader.Passes)
        {
            for (const auto& [pipelineStateName, pipelineStateValue] : dzPass.PipelineStates)
            {
                ProcessPipelineState(pipelineStateName, pipelineStateValue, pipelineState);
            }

            g_EngineContext->RenderSys->GetAPI()->SetPipelineState(pipelineState);
            DzShaderManager::BindPassResources(dzPass);

            int resourceBinding = 0;
            RenderMeshItem(sky, registry, camera.SkyboxCubeMesh, dzPass, *camera.SkyboxMaterial, resourceBinding);
            DzShaderManager::UnbindPassResources(dzPass);
        }
    }

    void DataDrivenPipeline::RenderMeshItem(entt::entity    geometry,
                                            entt::registry& registry,
                                            MeshItem&       meshItem,
                                            const DzPass&   dzPass,
                                            DzMaterial&     material,
                                            int&            resourceBinding) const
    {
        SNOW_LEOPARD_PROFILE_FUNCTION

        auto shader = DzShaderManager::GetCompiledPassShader(dzPass.Name);

        shader->Bind();

        // Set uniforms
        shader->SetMat4("model", registry.get<TransformComponent>(geometry).GetTransform());
        shader->SetMat4("view", m_SceneUniform.ViewMatrix);
        shader->SetMat4("projection", m_SceneUniform.ProjectionMatrix);
        shader->SetFloat3("viewPos", m_SceneUniform.ViewPos);
        shader->SetFloat("time", Time::ElapsedTime);
        shader->SetFloat3("directionalLight.direction", m_SceneUniform.DirectionalLightDirection);
        shader->SetFloat("directionalLight.intensity", m_SceneUniform.DirectionalLightIntensity);
        shader->SetFloat3("directionalLight.color", m_SceneUniform.DirectionalLightColor);
        shader->SetMat4("lightSpaceMatrix", m_SceneUniform.LightSpaceMatrix);

        // Set material properties
        for (const auto& property : material.GetPropertyBlock().ShaderProperties)
        {
            SetMaterialProperties(property.Name, property.Name, property.Type, material, shader, resourceBinding);
        }

        DzShaderManager::UsePassResources(dzPass, shader, resourceBinding);

        // Bind vertex array
        if (meshItem.Data.VertexArray == nullptr)
        {
            meshItem.Data.VertexArray = g_EngineContext->RenderSys->GetAPI()->CreateVertexArray(meshItem);
        }
        if (meshItem.Skinned())
        {
            meshItem.Data.VertexArray->GetVertexBuffers()[0]->SetBufferData(meshItem.Data.Vertices);
        }
        meshItem.Data.VertexArray->Bind();

        // Draw
        g_EngineContext->RenderSys->GetAPI()->DrawIndexed(meshItem.Data.Indices.size());

        // Unbind resources
        meshItem.Data.VertexArray->Unbind();
        shader->Unbind();
    }

    void DataDrivenPipeline::RenderMeshItemInstanced(const std::vector<entt::entity>& groupEntities,
                                                     entt::registry&                  registry,
                                                     MeshItem&                        meshItem,
                                                     const DzShader&                  dzShader,
                                                     const DzPass&                    dzPass,
                                                     DzMaterial&                      material,
                                                     int&                             resourceBinding) const
    {
        SNOW_LEOPARD_PROFILE_FUNCTION

        auto shader = DzShaderManager::GetCompiledPassShader(dzPass.Name);

        shader->Bind();

        // Set uniforms
        shader->SetMat4("view", m_SceneUniform.ViewMatrix);
        shader->SetMat4("projection", m_SceneUniform.ProjectionMatrix);
        shader->SetFloat3("viewPos", m_SceneUniform.ViewPos);
        shader->SetFloat("time", Time::ElapsedTime);
        shader->SetFloat3("directionalLight.direction", m_SceneUniform.DirectionalLightDirection);
        shader->SetFloat("directionalLight.intensity", m_SceneUniform.DirectionalLightIntensity);
        shader->SetFloat3("directionalLight.color", m_SceneUniform.DirectionalLightColor);
        shader->SetMat4("lightSpaceMatrix", m_SceneUniform.LightSpaceMatrix);

        // Set non-instanced material properties
        for (const auto& property : material.GetPropertyBlock().ShaderProperties)
        {
            // Jump instanced properties
            auto it = std::find(
                dzShader.InstancedPropertyNames.begin(), dzShader.InstancedPropertyNames.end(), property.Name);
            if (it != dzShader.InstancedPropertyNames.end())
            {
                continue;
            }

            SetMaterialProperties(property.Name, property.Name, property.Type, material, shader, resourceBinding);

            DzShaderManager::UsePassResources(dzPass, shader, resourceBinding);
        }

        // Set instanced material properties
        for (size_t instanceID = 0; instanceID < groupEntities.size(); ++instanceID)
        {
            auto  geometry         = groupEntities[instanceID];
            auto& instanceRenderer = registry.get<MeshRendererComponent>(geometry);

            // Auto set material properties
            for (const auto& property : instanceRenderer.Material->GetPropertyBlock().ShaderProperties)
            {
                std::string shaderPropertyName = property.Name;
                auto        it                 = std::find(
                    dzShader.InstancedPropertyNames.begin(), dzShader.InstancedPropertyNames.end(), property.Name);

                // Now, only care about instanced material properties
                if (it != dzShader.InstancedPropertyNames.end())
                {
                    shaderPropertyName = fmt::format("{0}[{1}]", shaderPropertyName, instanceID);
                }
                else
                {
                    continue;
                }

                SetMaterialProperties(property.Name,
                                      shaderPropertyName,
                                      property.Type,
                                      *instanceRenderer.Material,
                                      shader,
                                      resourceBinding);
                DzShaderManager::UsePassResources(dzPass, shader, resourceBinding);
            }
        }

        // Bind vertex array
        if (meshItem.Data.VertexArray == nullptr)
        {
            meshItem.Data.VertexArray = g_EngineContext->RenderSys->GetAPI()->CreateVertexArray(meshItem);
        }
        if (meshItem.Skinned())
        {
            meshItem.Data.VertexArray->GetVertexBuffers()[0]->SetBufferData(meshItem.Data.Vertices);
        }
        meshItem.Data.VertexArray->Bind();

        // Draw
        g_EngineContext->RenderSys->GetAPI()->DrawIndexedInstanced(meshItem.Data.Indices.size(), groupEntities.size());

        // Unbind resources
        meshItem.Data.VertexArray->Unbind();
        shader->Unbind();
    }

    void DataDrivenPipeline::ProcessPipelineState(const std::string& pipelineStateName,
                                                  const std::string& pipelineStateValue,
                                                  PipelineState&     pipelineState)
    {
        SNOW_LEOPARD_PROFILE_FUNCTION

        if (pipelineStateName == "CullFaceMode")
        {
            auto optional          = magic_enum::enum_cast<CullFaceMode>(pipelineStateValue);
            pipelineState.CullFace = optional.has_value() ? optional.value() : CullFaceMode::Back;
        }

        if (pipelineStateName == "DepthTestMode")
        {
            auto optional           = magic_enum::enum_cast<DepthTestMode>(pipelineStateValue);
            pipelineState.DepthTest = optional.has_value() ? optional.value() : DepthTestMode::Less;
        }

        if (pipelineStateName == "DepthWrite")
        {
            pipelineState.DepthWrite = pipelineStateValue == "On";
        }

        if (pipelineStateName == "BlendMode")
        {
            auto optional       = magic_enum::enum_cast<BlendMode>(pipelineStateValue);
            pipelineState.Blend = optional.has_value() ? optional.value() : BlendMode::Enable;
        }

        if (pipelineStateName == "BlendFunc1")
        {
            auto optional            = magic_enum::enum_cast<BlendFunc>(pipelineStateValue);
            pipelineState.BlendFunc1 = optional.has_value() ? optional.value() : BlendFunc::SrcAlpha;
        }

        if (pipelineStateName == "BlendFunc2")
        {
            auto optional            = magic_enum::enum_cast<BlendFunc>(pipelineStateValue);
            pipelineState.BlendFunc2 = optional.has_value() ? optional.value() : BlendFunc::OneMinusSrcAlpha;
        }
    }

    void DataDrivenPipeline::SetMaterialProperties(const std::string& propertyName,
                                                   const std::string& shaderPropertyName,
                                                   const std::string& propertyType,
                                                   DzMaterial&        material,
                                                   const Ref<Shader>& shader,
                                                   int&               resourceBinding)
    {
        SNOW_LEOPARD_PROFILE_FUNCTION

        if (propertyType == "Int")
        {
            auto value = material.GetInt(propertyName);
            shader->SetInt(shaderPropertyName, value);
        }
        else if (propertyType == "Float")
        {
            auto value = material.GetFloat(propertyName);
            shader->SetFloat(shaderPropertyName, value);
        }
        else if (propertyType == "Color")
        {
            auto value = material.GetColor(propertyName);
            shader->SetFloat4(shaderPropertyName, value);
        }
        else if (propertyType == "Vector")
        {
            auto value = material.GetVector(propertyName);
            shader->SetFloat4(shaderPropertyName, value);
        }
        else if (propertyType == "Texture2D")
        {
            auto texture = material.GetTexture2D(propertyName);
            shader->SetInt(shaderPropertyName, resourceBinding);
            texture->Bind(resourceBinding);
            resourceBinding++;
        }
        else if (propertyType == "Cubemap")
        {
            auto cubemap = material.GetCubemap(propertyName);
            shader->SetInt(shaderPropertyName, resourceBinding);
            cubemap->Bind(resourceBinding);
            resourceBinding++;
        }
    }
} // namespace SnowLeopardEngine