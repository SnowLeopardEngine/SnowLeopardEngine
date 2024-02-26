#include "SnowLeopardEngine/Function/Rendering/Forward/ForwardShadowSubPass.h"
#include "SnowLeopardEngine/Engine/EngineContext.h"
#include "SnowLeopardEngine/Function/Rendering/Forward/ForwardSinglePass.h"
#include "SnowLeopardEngine/Function/Rendering/GraphicsAPI.h"
#include "SnowLeopardEngine/Function/Rendering/Pipeline/Pipeline.h"
#include "SnowLeopardEngine/Function/Scene/Components.h"

namespace SnowLeopardEngine
{
    void ForwardShadowSubPass::Draw()
    {
        auto* ownerPass = static_cast<ForwardSinglePass*>(m_OwnerPass);

        // Get pipeline
        auto* pipeline = ownerPass->GetPipeline();

        // Get pipeline state & set
        auto pipelineState = pipeline->GetStateManager()->GetState("ShadowMapping");
        pipeline->GetAPI()->SetPipelineState(pipelineState);

        // Load / Create shader
        m_Shader = pipeline->GetStateManager()->GetShaderProgram(pipelineState);

        // Get active logic scene
        auto activeScene = g_EngineContext->SceneMngr->GetActiveScene();
        if (activeScene == nullptr)
        {
            return;
        }

        auto& registry = activeScene->GetRegistry();

        // Get camera component, currently we pick the first one as main camera.
        // TODO: filter main camera & other cameras
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
                    isFirst                  = false;
                    mainCameraTransform      = transform;
                    mainCamera               = camera;
                }
                else
                {
                    break;
                }
            }
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

        ownerPass->GetShadowDepthBuffer()->Bind();
        pipeline->GetAPI()->ClearColor(glm::vec4(0, 0, 0, 0), ClearBit::Depth);

        // TODO: View frustum get AABB and set borders.
        glm::mat4 lightProjection  = glm::ortho(-150.0f, 150.0f, -150.0f, 150.0f, 1.0f, 10000.0f);
        auto      lightPos         = -1000.0f * directionalLight.Direction; // simulate directional light position
        glm::mat4 lightView        = glm::lookAt(lightPos, glm::vec3(0, 0, 0), glm::vec3(0.0f, 1.0f, 0.0f));
        glm::mat4 lightSpaceMatrix = lightProjection * lightView;

        // for each mesh in the scene, request draw call.
        registry.view<TransformComponent, MeshFilterComponent>().each(
            [this, pipeline, lightSpaceMatrix](TransformComponent& transform, MeshFilterComponent& meshFilter) {
                // No meshes, skip...
                if (meshFilter.Meshes.Items.empty())
                {
                    return;
                }

                for (const auto& meshItem : meshFilter.Meshes.Items)
                {
                    m_Shader->Bind();

                    m_Shader->SetMat4("lightSpaceMatrix", lightSpaceMatrix);
                    m_Shader->SetMat4("model", transform.GetTransform());

                    // Currently, no static batching.leave temp test code here auto vertexArray =
                    auto vertexArray = pipeline->GetAPI()->CreateVertexArray(meshItem);
                    vertexArray->Bind();

                    pipeline->GetAPI()->DrawIndexed(meshItem.Data.Indices.size());

                    vertexArray->Unbind();

                    m_Shader->Unbind();
                }
            });

        // for each terrain in the scene, request draw call.
        registry.view<TransformComponent, TerrainComponent>().each(
            [this, pipeline, lightSpaceMatrix](TransformComponent& transform, TerrainComponent& terrain) {
                // No heightmap, skip...
                if (terrain.TerrainHeightMap.Data.empty())
                {
                    return;
                }

                m_Shader->Bind();

                m_Shader->SetMat4("lightSpaceMatrix", lightSpaceMatrix);
                m_Shader->SetMat4("model", transform.GetTransform());

                // Currently, no static batching.leave temp test code here auto vertexArray =
                auto vertexArray = pipeline->GetAPI()->CreateVertexArray(terrain.Mesh);
                vertexArray->Bind();

                pipeline->GetAPI()->DrawIndexed(terrain.Mesh.Data.Indices.size());

                vertexArray->Unbind();

                m_Shader->Unbind();
            });
        ownerPass->GetShadowDepthBuffer()->Unbind();
    }
} // namespace SnowLeopardEngine