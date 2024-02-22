#include "SnowLeopardEngine/Function/Rendering/Forward/ForwardGeometrySubPass.h"
#include "SnowLeopardEngine/Engine/EngineContext.h"
#include "SnowLeopardEngine/Function/Rendering/Pass/RenderPass.h"
#include "SnowLeopardEngine/Function/Rendering/Pipeline/Pipeline.h"
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

        // for each mesh in the scene, request draw call.
        registry.view<TransformComponent, MeshFilterComponent, MeshRendererComponent>().each(
            [this, pipeline, mainCameraTransform, mainCamera](
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
                m_Shader->SetFloat3("lightPos", glm::vec3(-10, 20, 10));
                m_Shader->SetFloat3("viewPos", mainCameraTransform.Position);

                // Currently, no static batching. leave temp test code here
                auto vertexArray = pipeline->GetAPI()->CreateVertexArray(meshFilter.Meshes.Items[0]);
                vertexArray->Bind();

                pipeline->GetAPI()->DrawIndexed(meshFilter.Meshes.Items[0].Data.Indices.size());

                vertexArray->Unbind();
                m_Shader->Unbind();
            });
    }
} // namespace SnowLeopardEngine