#include "SnowLeopardEngine/Function/Rendering/Forward/ForwardGeometrySubPass.h"
#include "SnowLeopardEngine/Engine/EngineContext.h"
#include "SnowLeopardEngine/Function/Rendering/Pass/RenderPass.h"
#include "SnowLeopardEngine/Function/Rendering/Pipeline/Pipeline.h"
#include "SnowLeopardEngine/Function/Rendering/RHI/Texture.h"
#include "SnowLeopardEngine/Function/Scene/Components.h"

namespace SnowLeopardEngine
{
    void ForwardGeometrySubPass::Draw()
    {
        // Get pipeline
        auto* pipeline = m_OwnerPass->GetPipeline();

        // Get pipeline state & set
        auto pipelineState = pipeline->GetStateManager()->GetState("Phong");
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
                    pipeline->GetAPI()->ClearColor(camera.ClearColor);
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

        // for each mesh in the scene, request draw call.
        registry.view<TransformComponent, MeshFilterComponent, MeshRendererComponent>().each(
            [this, pipeline, mainCameraTransform, mainCamera, directionalLight](
                TransformComponent& transform, MeshFilterComponent& meshFilter, MeshRendererComponent& meshRenderer) {
                // No meshes, skip...
                if (meshFilter.Meshes.Items.empty())
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

                for (const auto& meshItem : meshFilter.Meshes.Items)
                {
                    m_Shader->Bind();
                    m_Shader->SetMat4("model", transform.GetTransform());
                    m_Shader->SetMat4(
                        "view", glm::lookAt(mainCameraTransform.Position, mainCameraTransform.Position + forward, up));
                    m_Shader->SetMat4("projection",
                                      glm::perspective(glm::radians(mainCamera.FOV),
                                                       viewPortDesc.Width / viewPortDesc.Height,
                                                       mainCamera.Near,
                                                       mainCamera.Far));
                    m_Shader->SetFloat4("baseColor", meshRenderer.BaseColor);
                    m_Shader->SetFloat3("viewPos", mainCameraTransform.Position);
                    m_Shader->SetFloat3("directionalLight.direction", directionalLight.Direction);
                    m_Shader->SetFloat("directionalLight.intensity", directionalLight.Intensity);
                    m_Shader->SetFloat3("directionalLight.color", directionalLight.Color);

                    // Bind diffuse texture
                    if (meshRenderer.UseDiffuse && meshRenderer.DiffuseTexture != nullptr)
                    {
                        meshRenderer.DiffuseTexture->Bind(0);
                        m_Shader->SetInt("diffuseMap", 0);
                        m_Shader->SetInt("useDiffuse", 1);
                    }
                    else
                    {
                        m_Shader->SetInt("useDiffuse", 0);
                    }

                    // Currently, no static batching.leave temp test code here auto vertexArray =
                    auto vertexArray = pipeline->GetAPI()->CreateVertexArray(meshItem);
                    vertexArray->Bind();

                    pipeline->GetAPI()->DrawIndexed(meshItem.Data.Indices.size());

                    vertexArray->Unbind();

                    m_Shader->Unbind();
                }
            });
    }
} // namespace SnowLeopardEngine