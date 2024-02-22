#include "SnowLeopardEngine/Function/Rendering/Forward/ForwardSkyboxSubPass.h"
#include "SnowLeopardEngine/Engine/EngineContext.h"
#include "SnowLeopardEngine/Function/Geometry/GeometryFactory.h"
#include "SnowLeopardEngine/Function/Rendering/Pass/RenderPass.h"
#include "SnowLeopardEngine/Function/Rendering/Pipeline/Pipeline.h"
#include "SnowLeopardEngine/Function/Scene/Components.h"

namespace SnowLeopardEngine
{
    void ForwardSkyboxSubPass::Draw()
    {
        // Get pipeline
        auto* pipeline = m_OwnerPass->GetPipeline();

        // Get pipeline state & set
        auto pipelineState = pipeline->GetStateManager()->GetState("Skybox");
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
        TransformComponent defaultTransform;
        m_Shader->SetMat4("model", defaultTransform.GetTransform());
        m_Shader->SetMat4("view",
                          glm::lookAt(mainCameraTransform.Position, mainCameraTransform.Position + forward, up));
        m_Shader->SetMat4("projection",
                          glm::perspective(glm::radians(mainCamera.FOV),
                                           viewPortDesc.Width / viewPortDesc.Height,
                                           mainCamera.Near,
                                           mainCamera.Far));

        auto meshItem    = GeometryFactory::CreateMeshPrimitive<CubeMesh>();
        auto vertexArray = pipeline->GetAPI()->CreateVertexArray(meshItem);
        vertexArray->Bind();

        pipeline->GetAPI()->DrawIndexed(meshItem.Data.Indices.size());

        vertexArray->Unbind();
        m_Shader->Unbind();
    }
} // namespace SnowLeopardEngine