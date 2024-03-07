#include "SnowLeopardEngine/Function/Rendering/Forward/ForwardSkyboxSubPass.h"
#include "SnowLeopardEngine/Core/Profiling/Profiling.h"
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
        SNOW_LEOPARD_PROFILE_FUNCTION
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

        auto viewPortDesc      = pipeline->GetAPI()->GetViewport();
        mainCamera.AspectRatio = viewPortDesc.Width / viewPortDesc.Height;

        m_Shader->Bind();

        auto viewMatrix       = g_EngineContext->CameraSys->GetViewMatrix(mainCameraTransform);
        auto projectionMatrix = g_EngineContext->CameraSys->GetProjectionMatrix(mainCamera);
        auto view             = glm::mat4(glm::mat3(viewMatrix));
        auto vp               = projectionMatrix * view;
        m_Shader->SetMat4("VP", vp);

        // Bind cubemap
        mainCamera.Cubemap->Bind(0);
        m_Shader->SetInt("cubeMap", 0);

        // lazy load
        if (m_SkyboxCubeMesh.Data.VertexArray == nullptr)
        {
            m_SkyboxCubeMesh.Data.VertexArray = pipeline->GetAPI()->CreateVertexArray(m_SkyboxCubeMesh);
        }
        m_SkyboxCubeMesh.Data.VertexArray->Bind();

        pipeline->GetAPI()->DrawIndexed(m_SkyboxCubeMesh.Data.Indices.size());

        m_SkyboxCubeMesh.Data.VertexArray->Unbind();

        m_Shader->Unbind();

        if (rt != nullptr)
        {
            rt->Unbind();
        }
    }
} // namespace SnowLeopardEngine