#include "SnowLeopardEngine/Function/Rendering/Forward/ForwardGeometrySubPass.h"
#include "SnowLeopardEngine/Engine/EngineContext.h"
#include "SnowLeopardEngine/Function/Rendering/Forward/ForwardSinglePass.h"
#include "SnowLeopardEngine/Function/Rendering/GraphicsAPI.h"
#include "SnowLeopardEngine/Function/Rendering/Pipeline/Pipeline.h"
#include "SnowLeopardEngine/Function/Rendering/RHI/Texture.h"
#include "SnowLeopardEngine/Function/Scene/Components.h"

namespace SnowLeopardEngine
{
    void ForwardGeometrySubPass::Draw()
    {
        auto* ownerPass = static_cast<ForwardSinglePass*>(m_OwnerPass);

        // Get pipeline
        auto* pipeline = ownerPass->GetPipeline();

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

        // If RT is set, render to RT.
        auto rt = pipeline->GetRenderTarget();
        if (rt != nullptr)
        {
            rt->Bind();
        }

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
                    pipeline->GetAPI()->ClearColor(camera.ClearColor, ClearBit::Default);
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

        // after shadow-mapping, update the viewport use the size of the window, if RT is not set
        if (rt == nullptr)
        {
            pipeline->GetAPI()->UpdateViewport(
                0, 0, g_EngineContext->WindowSys->GetWidth(), g_EngineContext->WindowSys->GetHeight());
        }
        else
        {
            pipeline->GetAPI()->UpdateViewport(0, 0, rt->GetDesc().Width, rt->GetDesc().Height);
        }

        auto viewPortDesc      = pipeline->GetAPI()->GetViewport();
        mainCamera.AspectRatio = viewPortDesc.Width / viewPortDesc.Height;

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

        // for each mesh in the scene, request draw call.
        registry.view<TransformComponent, MeshFilterComponent, MeshRendererComponent>().each(
            [this, pipeline, mainCameraTransform, mainCamera, directionalLight, ownerPass, lightSpaceMatrix](
                TransformComponent& transform, MeshFilterComponent& meshFilter, MeshRendererComponent& meshRenderer) {
                // No meshes, skip...
                if (meshFilter.Meshes.Items.empty())
                {
                    return;
                }

                for (const auto& meshItem : meshFilter.Meshes.Items)
                {
                    m_Shader->Bind();
                    m_Shader->SetMat4("model", transform.GetTransform());
                    m_Shader->SetMat4("view", g_EngineContext->CameraSys->GetViewMatrix(mainCameraTransform));
                    m_Shader->SetMat4("projection", g_EngineContext->CameraSys->GetProjectionMatrix(mainCamera));
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

                    // Bind shadow map
                    m_Shader->SetMat4("lightSpaceMatrix", lightSpaceMatrix);
                    ownerPass->GetShadowDepthBuffer()->BindDepthAttachmentTexture(1);
                    m_Shader->SetInt("shadowMap", 1);
                    m_Shader->SetInt("castShadow", meshRenderer.CastShadow);

                    // Currently, no static batching.leave temp test code here auto vertexArray =
                    auto vertexArray = pipeline->GetAPI()->CreateVertexArray(meshItem);
                    vertexArray->Bind();

                    pipeline->GetAPI()->DrawIndexed(meshItem.Data.Indices.size());

                    vertexArray->Unbind();

                    m_Shader->Unbind();
                }
            });

        if (rt != nullptr)
        {
            rt->Unbind();
        }
    }
} // namespace SnowLeopardEngine