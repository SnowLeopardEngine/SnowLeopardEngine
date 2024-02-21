#include "SnowLeopardEngine/Function/Rendering/Forward/ForwardSinglePass.h"
#include "SnowLeopardEngine/Core/File/FileSystem.h"
#include "SnowLeopardEngine/Core/Log/LogSystem.h"
#include "SnowLeopardEngine/Engine/EngineContext.h"
#include "SnowLeopardEngine/Function/Rendering/Forward/ForwardPipeline.h"
#include "SnowLeopardEngine/Function/Rendering/RHI/Shader.h"
#include "SnowLeopardEngine/Function/Scene/Components.h"

#include <fmt/core.h>

namespace SnowLeopardEngine
{
    void ForwardSinglePass::Draw()
    {
        // Get pipeline state & set
        auto pipelineState = m_Pipeline->GetStateManager()->GetState("Forward");
        m_Pipeline->GetAPI()->SetPipelineState(pipelineState);

        // Load / Create shader
        m_Shader = m_Pipeline->GetStateManager()->GetShaderProgram(pipelineState);

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
                    m_Pipeline->GetAPI()->ClearColor(camera.ClearColor);
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
            [this, mainCameraTransform, mainCamera](
                TransformComponent& transform, MeshFilterComponent& meshFilter, MeshRendererComponent& meshRenderer) {
                // No meshes, skip...
                if (meshFilter.Meshes.Items.empty())
                {
                    return;
                }

                auto viewPortDesc = m_Pipeline->GetAPI()->GetViewport();

                // Setup camera matrices
                auto eulerAngles = mainCameraTransform.GetRotationEuler();

                // Calculate forward (Pitch - 90 to adjust)
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

                // Currently, no static batching. leave temp test code here
                auto vertexArray = m_Pipeline->GetAPI()->CreateVertexArray(meshFilter.Meshes.Items[0]);
                vertexArray->Bind();

                m_Pipeline->GetAPI()->DrawIndexed(meshFilter.Meshes.Items[0].Data.Indices.size());

                vertexArray->Unbind();
                m_Shader->Unbind();
            });
    }
} // namespace SnowLeopardEngine