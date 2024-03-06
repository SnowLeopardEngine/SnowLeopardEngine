#include "SnowLeopardEngine/Function/Rendering/RenderSystem.h"
#include "SnowLeopardEngine/Core/Base/Base.h"
#include "SnowLeopardEngine/Core/Profiling/Profiling.h"
#include "SnowLeopardEngine/Engine/EngineContext.h"
#include "SnowLeopardEngine/Function/Rendering/DzShader/DzShaderManager.h"
#include "SnowLeopardEngine/Function/Rendering/Forward/ForwardPipeline.h"
#include "SnowLeopardEngine/Function/Rendering/GraphicsAPI.h"
#include "SnowLeopardEngine/Function/Rendering/Pipeline/Pipeline.h"
#include "SnowLeopardEngine/Function/Scene/Components.h"

namespace SnowLeopardEngine
{
    RenderSystem::RenderSystem()
    {
        m_Context = GraphicsContext::Create();
        m_Context->Init();

        m_API = GraphicsAPI::Create(GraphicsBackend::OpenGL);

        PipelineInitInfo pipelineInitInfo = {};
        pipelineInitInfo.API              = m_API;
        m_Pipeline                        = CreateRef<ForwardPipeline>();
        m_Pipeline->Init(pipelineInitInfo);

        // TODO: Remove old pipeline & pass code. Totally switch to data-driven (shader-driven) pipeline.

        SNOW_LEOPARD_CORE_INFO("[RenderSystem] Initialized");
        m_State = SystemState::InitOk;
    }

    RenderSystem::~RenderSystem()
    {
        SNOW_LEOPARD_CORE_INFO("[RenderSystem] Shutting Down...");

        m_Pipeline->Shutdown();

        m_Context->Shutdown();
        m_Context.reset();

        m_State = SystemState::ShutdownOk;
    }

    void RenderSystem::OnTick(float deltaTime)
    {
        SNOW_LEOPARD_PROFILE_FUNCTION
        // Draw Built-in Deferred Pipeline
        // Now, draw forward instead for testing
        // m_Pipeline->Tick(deltaTime);

        if (!m_LoadedScene)
        {
            // https://docs.unity3d.com/ScriptReference/Rendering.RenderQueue.html
            // 1. Filter materials (shaders) that have the same render queue priority. (e.g. Geometry, Transparent)
            //    Divide entities into different groups by material.

            // 2. For each type of material in groups, compile relevant shaders.

            // TODO: Clean code
            auto scene = g_EngineContext->SceneMngr->GetActiveScene();
            if (scene != nullptr)
            {
                auto& registry = scene->GetRegistry();

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

                m_LoadedScene = true;
            }
        }

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

        auto  scene    = g_EngineContext->SceneMngr->GetActiveScene();
        auto& registry = scene->GetRegistry();

        TransformComponent mainCameraTransform;
        CameraComponent    mainCamera;
        {
            bool isFirst = true;
            auto view    = registry.view<TransformComponent, CameraComponent>();

            // No Camera, return
            if (view.size_hint() == 0)
            {
                return;
            }

            for (const auto& cameraEntity : view)
            {
                if (isFirst)
                {
                    auto [transform, camera] = view.get<TransformComponent, CameraComponent>(cameraEntity);
                    m_API->ClearColor(camera.ClearColor, ClearBit::Default);
                    isFirst             = false;
                    mainCameraTransform = transform;
                    mainCamera          = camera;
                }
                else
                {
                    break;
                }
            }
        }

        for (auto& geometry : m_GeometryGroup)
        {
            auto& transform    = registry.get<TransformComponent>(geometry);
            auto& meshFilter   = registry.get<MeshFilterComponent>(geometry);
            auto& meshRenderer = registry.get<MeshRendererComponent>(geometry);
            auto  dzShaderName = meshRenderer.Material->GetShaderName();
            auto& dzShader     = DzShaderManager::GetShader(dzShaderName);

            if (meshFilter.Meshes.Items.empty() || meshRenderer.Material == nullptr)
            {
                continue;
            }

            for (const auto& [pipelineStateName, pipelineStateValue] : dzShader.PipelineStates)
            {
                // Ignore for now, need to update pipeline state manager
            }

            for (const auto& dzPass : dzShader.Passes)
            {
                for (const auto& [pipelineStateName, pipelineStateValue] : dzPass.PipelineStates)
                {
                    // Ignore for now, need to update pipeline state manager
                }

                int  textureLocator = 0;
                auto shader         = DzShaderManager::GetCompiledPassShader(dzPass.Name);
                shader->Bind();

                shader->SetMat4("model", transform.GetTransform());
                shader->SetMat4("view", g_EngineContext->CameraSys->GetViewMatrix(mainCameraTransform));
                shader->SetMat4("projection", g_EngineContext->CameraSys->GetProjectionMatrix(mainCamera));

                // Auto set material properties
                for (const auto& property : meshRenderer.Material->GetPropertyBlock().ShaderProperties)
                {
                    if (property.Type == "Int")
                    {
                        auto value = meshRenderer.Material->GetInt(property.Name);
                        shader->SetInt(property.Name, value);
                    }
                    else if (property.Type == "Float")
                    {
                        auto value = meshRenderer.Material->GetFloat(property.Name);
                        shader->SetFloat(property.Name, value);
                    }
                    else if (property.Type == "Color")
                    {
                        auto value = meshRenderer.Material->GetColor(property.Name);
                        shader->SetFloat4(property.Name, value);
                    }
                    else if (property.Type == "Texture2D")
                    {
                        auto texture = meshRenderer.Material->GetTexture(property.Name);
                        shader->SetInt(property.Name, textureLocator);
                        texture->Bind(textureLocator);
                        textureLocator++;
                    }
                }

                auto& meshItem = meshFilter.Meshes.Items[0];

                // lazy load
                if (meshItem.Data.VertexArray == nullptr)
                {
                    meshItem.Data.VertexArray = m_API->CreateVertexArray(meshItem);
                }
                meshItem.Data.VertexArray->Bind();

                m_API->DrawIndexed(meshItem.Data.Indices.size());

                meshItem.Data.VertexArray->Unbind();

                shader->Unbind();
            }
        }
    }

    void RenderSystem::Present() { m_Context->SwapBuffers(); }
} // namespace SnowLeopardEngine