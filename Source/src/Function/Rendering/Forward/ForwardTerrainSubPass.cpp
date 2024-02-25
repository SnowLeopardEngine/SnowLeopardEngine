#include "SnowLeopardEngine/Function/Rendering/Forward/ForwardTerrainSubPass.h"
#include "SnowLeopardEngine/Engine/EngineContext.h"
#include "SnowLeopardEngine/Function/Rendering/Pass/RenderPass.h"
#include "SnowLeopardEngine/Function/Rendering/Pipeline/Pipeline.h"
#include "SnowLeopardEngine/Function/Scene/Components.h"

namespace SnowLeopardEngine
{
    void ForwardTerrainSubPass::Draw()
    {
        // Get pipeline
        auto* pipeline = m_OwnerPass->GetPipeline();

        // Get pipeline state & set
        auto pipelineState = pipeline->GetStateManager()->GetState("Terrain");
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

        // TODO: Ziyu Min

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

        // for each terrain in the scene, request draw call.
        registry.view<TransformComponent, TerrainComponent, TerrainRendererComponent>().each(
            [this, pipeline, mainCameraTransform, mainCamera](
                TransformComponent& transform, TerrainComponent& terrain, TerrainRendererComponent& terrainRenderer) {
                // No heightmap, skip...
                if (terrain.TerrainHeightMap.Data.empty())
                {
                    return;
                }

                auto viewPortDesc = pipeline->GetAPI()->GetViewport();

                // Setup camera matrices
                auto eulerAngles = mainCameraTransform.GetRotationEuler();

                // Calculate forward (Yaw - 90 to adjust)
                glm::vec3 forward;
                forward.x = cos(glm::radians(eulerAngles.y - 90)) * cos(glm::radians(eulerAngles.x));
                forward.y = sin(glm::radians(eulerAngles.x));
                forward.z = sin(glm::radians(eulerAngles.y - 90)) * cos(glm::radians(eulerAngles.x));
                forward   = glm::normalize(forward);

                // Calculate up
                glm::vec3 right = glm::normalize(glm::cross(forward, glm::vec3(0, 1, 0)));
                glm::vec3 up    = glm::normalize(glm::cross(right, forward));

                m_Shader->Bind();
                m_Shader->SetMat4("model", transform.GetTransform());
                m_Shader->SetMat4(
                    "view", glm::lookAt(mainCameraTransform.Position, mainCameraTransform.Position + forward, up));
                m_Shader->SetMat4("projection",
                                  glm::perspective(glm::radians(mainCamera.FOV),
                                                   viewPortDesc.Width / viewPortDesc.Height,
                                                   mainCamera.Near,
                                                   mainCamera.Far));
                m_Shader->SetFloat4("baseColor", terrainRenderer.BaseColor);
                m_Shader->SetFloat3("lightPos", glm::vec3(-10, 20, 10));
                m_Shader->SetFloat3("viewPos", mainCameraTransform.Position);

                // Bind diffuse texture
                if (terrainRenderer.UseDiffuse && terrainRenderer.DiffuseTexture != nullptr)
                {
                    terrainRenderer.DiffuseTexture->Bind(0);
                    m_Shader->SetInt("diffuseMap", 0);
                    m_Shader->SetInt("useDiffuse", 1);
                }
                else
                {
                    m_Shader->SetInt("useDiffuse", 0);
                }

                // Currently, no static batching. leave temp test code here
                auto vertexArray = pipeline->GetAPI()->CreateVertexArray(terrain.Mesh);
                vertexArray->Bind();

                pipeline->GetAPI()->DrawIndexed(terrain.Mesh.Data.Indices.size());

                vertexArray->Unbind();
                m_Shader->Unbind();
            });
    }
} // namespace SnowLeopardEngine