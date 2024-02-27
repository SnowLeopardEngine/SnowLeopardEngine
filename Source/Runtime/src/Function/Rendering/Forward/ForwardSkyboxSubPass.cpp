#include "SnowLeopardEngine/Function/Rendering/Forward/ForwardSkyboxSubPass.h"
#include "SnowLeopardEngine/Engine/EngineContext.h"
#include "SnowLeopardEngine/Function/Geometry/GeometryFactory.h"
#include "SnowLeopardEngine/Function/Rendering/Forward/ForwardSinglePass.h"
#include "SnowLeopardEngine/Function/Rendering/Pipeline/Pipeline.h"
#include "SnowLeopardEngine/Function/Scene/Components.h"

namespace SnowLeopardEngine
{
    ForwardSkyboxSubPass::ForwardSkyboxSubPass()
    {
        m_SkyboxCubeMesh = GeometryFactory::CreateMeshPrimitive<CubeMesh>();
    }

    void ForwardSkyboxSubPass::Draw()
    {
        auto* ownerPass = static_cast<ForwardSinglePass*>(m_OwnerPass);

        // Get pipeline
        auto* pipeline = ownerPass->GetPipeline();

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

        // If RT is set, render to RT.
        auto rt = pipeline->GetRenderTarget();
        if (rt != nullptr)
        {
            rt->Bind();
        }

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

                    // Skybox not enabled
                    if (camera.ClearFlags != CameraClearFlags::Skybox)
                    {
                        return;
                    }

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

        auto viewMatrix       = glm::lookAt(mainCameraTransform.Position, mainCameraTransform.Position + forward, up);
        auto projectionMatrix = glm::perspective(
            glm::radians(mainCamera.FOV), viewPortDesc.Width / viewPortDesc.Height, mainCamera.Near, mainCamera.Far);
        auto view = glm::mat4(glm::mat3(viewMatrix));
        auto vp   = projectionMatrix * view;
        m_Shader->SetMat4("VP", vp);

        // Bind cubemap
        mainCamera.Cubemap->Bind(0);
        m_Shader->SetInt("cubeMap", 0);

        auto vertexArray = pipeline->GetAPI()->CreateVertexArray(m_SkyboxCubeMesh);
        vertexArray->Bind();

        pipeline->GetAPI()->DrawIndexed(m_SkyboxCubeMesh.Data.Indices.size());

        vertexArray->Unbind();
        m_Shader->Unbind();

        if (rt != nullptr)
        {
            rt->Unbind();
        }
    }
} // namespace SnowLeopardEngine