#include "SnowLeopardEngine/Function/Rendering/RenderSystem.h"
#include "SnowLeopardEngine/Core/Base/Base.h"
#include "SnowLeopardEngine/Core/Profiling/Profiling.h"
#include "SnowLeopardEngine/Engine/EngineContext.h"
#include "SnowLeopardEngine/Function/Rendering/DzShader/DzShaderManager.h"
#include "SnowLeopardEngine/Function/Rendering/GraphicsAPI.h"
#include "SnowLeopardEngine/Function/Rendering/Pipeline/PipelineState.h"
#include "SnowLeopardEngine/Function/Rendering/RHI/FrameBuffer.h"
#include "SnowLeopardEngine/Function/Rendering/RHI/Shader.h"
#include "SnowLeopardEngine/Function/Scene/Components.h"
#include "entt/entity/entity.hpp"

namespace SnowLeopardEngine
{
    RenderSystem::RenderSystem()
    {
        m_Context = GraphicsContext::Create();
        m_Context->Init();

        // TODO: Configurable vsync
        // Disable VSync
        m_Context->SetVSync(false);

        m_API = GraphicsAPI::Create(GraphicsBackend::OpenGL);

        Subscribe(m_LogicSceneLoadedHandler);

        SNOW_LEOPARD_CORE_INFO("[RenderSystem] Initialized");
        m_State = SystemState::InitOk;
    }

    RenderSystem::~RenderSystem()
    {
        SNOW_LEOPARD_CORE_INFO("[RenderSystem] Shutting Down...");

        Unsubscribe(m_LogicSceneLoadedHandler);

        m_Context->Shutdown();
        m_Context.reset();

        m_State = SystemState::ShutdownOk;
    }

    void RenderSystem::OnTick(float deltaTime)
    {
        SNOW_LEOPARD_PROFILE_FUNCTION

        // TODO: Remove old pipeline & pass code. Totally switch to data-driven (shader-driven) pipeline.
        // Shader-driven Pipeline tick:

        // For each mesh group divided by material:
        //     For each mesh in the group:
        //         Get the relevant material & shader
        //         Setup per-shader pipeline states.
        //         For each pass in the shader:
        //             Setup per-pass pipeline states.
        //             Setup frame uniforms (MVP, CameraPos, ...)
        //             Setup shader properties from material

        // TODO: Clean code
        // Let's implement a simple GeometryPass through Phong.dzshader.

        auto scene = g_EngineContext->SceneMngr->GetActiveScene();
        if (scene == nullptr)
        {
            return;
        }

        auto& registry = scene->GetRegistry();

        TransformComponent mainCameraTransform;
        CameraComponent    mainCamera;
        auto               firstCam = registry.view<TransformComponent, CameraComponent>().front();
        if (firstCam != entt::null)
        {
            auto [transform, camera] = registry.view<TransformComponent, CameraComponent>().get(firstCam);
            m_API->ClearColor(camera.ClearColor, ClearBit::Default);
            mainCameraTransform = transform;
            mainCamera          = camera;
        }
        else
        {
            return;
        }

        // filter the first directional light
        DirectionalLightComponent directionalLight;
        {
            bool isFirst = true;
            auto view    = registry.view<TransformComponent, DirectionalLightComponent>();

            // No Directional light
            if (view.size_hint() == 0)
            {
                directionalLight.Intensity = 0;
            }

            for (const auto& directionalLightEntity : view)
            {
                if (isFirst)
                {
                    auto light       = view.get<DirectionalLightComponent>(directionalLightEntity);
                    isFirst          = false;
                    directionalLight = light;
                }
                else
                {
                    SNOW_LEOPARD_CORE_WARN("[Rendering][Forward][Geometry] There must be only 1 directional light!");
                    break;
                }
            }
        }

        // TODO: View frustum get AABB and set borders.
        glm::mat4 lightProjection  = glm::ortho(-150.0f, 150.0f, -150.0f, 150.0f, 1.0f, 10000.0f);
        auto      lightPos         = -1000.0f * directionalLight.Direction; // simulate directional light position
        glm::mat4 lightView        = glm::lookAt(lightPos, glm::vec3(0, 0, 0), glm::vec3(0.0f, 1.0f, 0.0f));
        glm::mat4 lightSpaceMatrix = lightProjection * lightView;

        if (directionalLight.ShadowMaterial != nullptr)
        {
            auto  shadowShaderName = directionalLight.ShadowMaterial->GetShaderName();
            auto& shadowShader     = DzShaderManager::GetShader(shadowShaderName);

            PipelineState pipelineState;

            for (const auto& [pipelineStateName, pipelineStateValue] : shadowShader.PipelineStates)
            {
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

                // TODO: Handle more pipeline states
            }

            for (auto& shadowPass : shadowShader.Passes)
            {
                for (const auto& [pipelineStateName, pipelineStateValue] : shadowPass.PipelineStates)
                {
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

                    // TODO: Handle more pipeline states
                }

                m_API->SetPipelineState(pipelineState);

                DzShaderManager::BindPassResources(shadowPass);

                int  resourceLocator = 0;
                auto shader          = DzShaderManager::GetCompiledPassShader(shadowPass.Name);

                for (auto& geometry : m_GeometryGroup)
                {
                    auto& transform = registry.get<TransformComponent>(geometry);

                    std::vector<MeshItem*> meshItemPtrs;

                    if (registry.any_of<MeshFilterComponent>(geometry))
                    {
                        auto& meshFilter = registry.get<MeshFilterComponent>(geometry);
                        for (auto& meshItem : meshFilter.Meshes.Items)
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
                        shader->Bind();

                        shader->SetMat4("model", transform.GetTransform());
                        shader->SetMat4("view", g_EngineContext->CameraSys->GetViewMatrix(mainCameraTransform));
                        shader->SetMat4("projection", g_EngineContext->CameraSys->GetProjectionMatrix(mainCamera));
                        shader->SetFloat3("viewPos", mainCameraTransform.Position);
                        shader->SetFloat3("directionalLight.direction", directionalLight.Direction);
                        shader->SetFloat("directionalLight.intensity", directionalLight.Intensity);
                        shader->SetFloat3("directionalLight.color", directionalLight.Color);
                        shader->SetMat4("lightSpaceMatrix", lightSpaceMatrix);

                        bool hasAnimation = meshItemPtr->Data.HasAnimationInfo();
                        shader->SetInt("hasAnimation", hasAnimation);

                        if (hasAnimation)
                        {
                            auto& animator     = registry.get<AnimatorComponent>(geometry);
                            auto  boneMatrices = animator.CurrentAnimator->GetFinalBoneMatrices();
                            for (uint32_t i = 0; i < boneMatrices.size(); ++i)
                            {
                                shader->SetMat4(fmt::format("finalBonesMatrices[{0}]", i), boneMatrices[i]);
                            }
                        }

                        // lazy load
                        if (meshItemPtr->Data.VertexArray == nullptr)
                        {
                            meshItemPtr->Data.VertexArray = m_API->CreateVertexArray(*meshItemPtr);
                        }
                        meshItemPtr->Data.VertexArray->Bind();

                        m_API->DrawIndexed(meshItemPtr->Data.Indices.size());

                        meshItemPtr->Data.VertexArray->Unbind();
                        shader->Unbind();
                    }
                }

                DzShaderManager::UnbindPassResources(shadowPass);
            }

            // After shadow mapping, restore view port size
            m_API->UpdateViewport(
                0, 0, g_EngineContext->WindowSys->GetWidth(), g_EngineContext->WindowSys->GetHeight());
            mainCamera.AspectRatio =
                g_EngineContext->WindowSys->GetWidth() * 1.0f / g_EngineContext->WindowSys->GetHeight();
        }

        if (m_RenderTarget != nullptr)
        {
            m_RenderTarget->Bind();

            auto     desc   = m_RenderTarget->GetDesc();
            uint32_t width  = desc.Width;
            uint32_t height = desc.Height;

            m_API->UpdateViewport(0, 0, width, height);
            mainCamera.AspectRatio = width * 1.0f / height;
        }

        m_API->ClearColor(mainCamera.ClearColor, ClearBit::Default);

        for (auto& geometry : m_GeometryGroup)
        {
            auto& transform = registry.get<TransformComponent>(geometry);

            std::vector<MeshItem*> meshItemPtrs;
            BaseRendererComponent  renderer;

            if (registry.any_of<MeshFilterComponent, MeshRendererComponent>(geometry))
            {
                auto& meshFilter = registry.get<MeshFilterComponent>(geometry);
                renderer         = registry.get<MeshRendererComponent>(geometry);
                for (auto& meshItem : meshFilter.Meshes.Items)
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

                // TODO: Handle more pipeline states
            }

            for (const auto& dzPass : dzShader.Passes)
            {
                for (const auto& [pipelineStateName, pipelineStateValue] : dzPass.PipelineStates)
                {
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

                    // TODO: Handle more pipeline states
                }

                m_API->SetPipelineState(pipelineState);

                DzShaderManager::BindPassResources(dzPass);

                int  resourceBinding = 0;
                auto shader          = DzShaderManager::GetCompiledPassShader(dzPass.Name);

                for (auto& meshItemPtr : meshItemPtrs)
                {
                    shader->Bind();

                    shader->SetMat4("model", transform.GetTransform());
                    shader->SetMat4("view", g_EngineContext->CameraSys->GetViewMatrix(mainCameraTransform));
                    shader->SetMat4("projection", g_EngineContext->CameraSys->GetProjectionMatrix(mainCamera));
                    shader->SetFloat3("viewPos", mainCameraTransform.Position);
                    shader->SetFloat3("directionalLight.direction", directionalLight.Direction);
                    shader->SetFloat("directionalLight.intensity", directionalLight.Intensity);
                    shader->SetFloat3("directionalLight.color", directionalLight.Color);
                    shader->SetMat4("lightSpaceMatrix", lightSpaceMatrix);

                    bool hasAnimation = meshItemPtr->Data.HasAnimationInfo();
                    shader->SetInt("hasAnimation", hasAnimation);

                    if (hasAnimation)
                    {
                        auto& animator     = registry.get<AnimatorComponent>(geometry);
                        auto  boneMatrices = animator.CurrentAnimator->GetFinalBoneMatrices();
                        for (uint32_t i = 0; i < boneMatrices.size(); ++i)
                        {
                            shader->SetMat4(fmt::format("finalBonesMatrices[{0}]", i), boneMatrices[i]);
                        }
                    }

                    // Auto set material properties
                    for (const auto& property : renderer.Material->GetPropertyBlock().ShaderProperties)
                    {
                        if (property.Type == "Int")
                        {
                            auto value = renderer.Material->GetInt(property.Name);
                            shader->SetInt(property.Name, value);
                        }
                        else if (property.Type == "Float")
                        {
                            auto value = renderer.Material->GetFloat(property.Name);
                            shader->SetFloat(property.Name, value);
                        }
                        else if (property.Type == "Color")
                        {
                            auto value = renderer.Material->GetColor(property.Name);
                            shader->SetFloat4(property.Name, value);
                        }
                        else if (property.Type == "Texture2D")
                        {
                            auto texture = renderer.Material->GetTexture2D(property.Name);
                            shader->SetInt(property.Name, resourceBinding);
                            texture->Bind(resourceBinding);
                            resourceBinding++;
                        }
                        else if (property.Type == "Cubemap")
                        {
                            auto cubemap = renderer.Material->GetCubemap(property.Name);
                            shader->SetInt(property.Name, resourceBinding);
                            cubemap->Bind(resourceBinding);
                            resourceBinding++;
                        }
                    }

                    DzShaderManager::UsePassResources(dzPass, shader, resourceBinding);

                    // lazy load
                    if (meshItemPtr->Data.VertexArray == nullptr)
                    {
                        meshItemPtr->Data.VertexArray = m_API->CreateVertexArray(*meshItemPtr);
                    }
                    meshItemPtr->Data.VertexArray->Bind();

                    m_API->DrawIndexed(meshItemPtr->Data.Indices.size());

                    meshItemPtr->Data.VertexArray->Unbind();

                    shader->Unbind();

                    DzShaderManager::UnbindPassResources(dzPass);
                }
            }
        }

        for (auto& sky : m_SkyGroup)
        {
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

                // TODO: Handle more pipeline states
            }

            for (const auto& dzPass : dzShader.Passes)
            {
                for (const auto& [pipelineStateName, pipelineStateValue] : dzPass.PipelineStates)
                {
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

                    // TODO: Handle more pipeline states
                }

                m_API->SetPipelineState(pipelineState);

                DzShaderManager::BindPassResources(dzPass);

                int  textureLocator = 0;
                auto shader         = DzShaderManager::GetCompiledPassShader(dzPass.Name);
                shader->Bind();

                shader->SetMat4("model", transform.GetTransform());
                shader->SetMat4("view", g_EngineContext->CameraSys->GetViewMatrix(mainCameraTransform));
                shader->SetMat4("projection", g_EngineContext->CameraSys->GetProjectionMatrix(mainCamera));

                // Auto set material properties
                for (const auto& property : camera.SkyboxMaterial->GetPropertyBlock().ShaderProperties)
                {
                    if (property.Type == "Int")
                    {
                        auto value = camera.SkyboxMaterial->GetInt(property.Name);
                        shader->SetInt(property.Name, value);
                    }
                    else if (property.Type == "Float")
                    {
                        auto value = camera.SkyboxMaterial->GetFloat(property.Name);
                        shader->SetFloat(property.Name, value);
                    }
                    else if (property.Type == "Color")
                    {
                        auto value = camera.SkyboxMaterial->GetColor(property.Name);
                        shader->SetFloat4(property.Name, value);
                    }
                    else if (property.Type == "Texture2D")
                    {
                        auto texture = camera.SkyboxMaterial->GetTexture2D(property.Name);
                        shader->SetInt(property.Name, textureLocator);
                        texture->Bind(textureLocator);
                        textureLocator++;
                    }
                    else if (property.Type == "Cubemap")
                    {
                        auto cubemap = camera.SkyboxMaterial->GetCubemap(property.Name);
                        shader->SetInt(property.Name, textureLocator);
                        cubemap->Bind(textureLocator);
                        textureLocator++;
                    }
                }

                DzShaderManager::UsePassResources(dzPass, shader, textureLocator);

                auto& meshItem = camera.SkyboxCubeMesh;

                // lazy load
                if (meshItem.Data.VertexArray == nullptr)
                {
                    meshItem.Data.VertexArray = m_API->CreateVertexArray(meshItem);
                }
                meshItem.Data.VertexArray->Bind();

                m_API->DrawIndexed(meshItem.Data.Indices.size());

                meshItem.Data.VertexArray->Unbind();

                shader->Unbind();

                DzShaderManager::UnbindPassResources(dzPass);
            }
        }

        if (m_RenderTarget != nullptr)
        {
            m_RenderTarget->Unbind();
        }
    }

    void RenderSystem::Present() { m_Context->SwapBuffers(); }

    void RenderSystem::OnLogicSceneLoaded(const LogicSceneLoadedEvent& e)
    {
        // https://docs.unity3d.com/ScriptReference/Rendering.RenderQueue.html
        // 1. Filter materials (shaders) that have the same render queue priority. (e.g. Geometry, Transparent)
        //    Divide entities into different groups by material.

        // 2. For each type of material in groups, compile relevant shaders.
        //    Setup shader resources (e.g. FrameBuffers)

        // TODO: Clean code
        auto* scene = e.GetLogicScene();
        if (scene != nullptr)
        {
            m_ShadowGroup.clear();
            m_GeometryGroup.clear();
            m_SkyGroup.clear();

            auto& registry = scene->GetRegistry();

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
                [this](
                    entt::entity entity, TransformComponent& transform, DirectionalLightComponent& directionalLight) {
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
    }
} // namespace SnowLeopardEngine