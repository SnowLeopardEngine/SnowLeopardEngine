#include "SnowLeopardEngine/Function/Rendering/Forward/ForwardTerrainSubPass.h"
#include "SnowLeopardEngine/Engine/EngineContext.h"
#include "SnowLeopardEngine/Function/Rendering/Forward/ForwardSinglePass.h"
#include "SnowLeopardEngine/Function/Rendering/Pipeline/Pipeline.h"
#include "SnowLeopardEngine/Function/Scene/Components.h"

namespace SnowLeopardEngine
{
    void ForwardTerrainSubPass::Draw()
    {
        auto* ownerPass = static_cast<ForwardSinglePass*>(m_OwnerPass);

        // Get pipeline
        auto* pipeline = ownerPass->GetPipeline();

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

        // TODO: View frustum get AABB and set borders.
        glm::mat4 lightProjection  = glm::ortho(-150.0f, 150.0f, -150.0f, 150.0f, 1.0f, 10000.0f);
        auto      lightPos         = -1000.0f * directionalLight.Direction; // simulate directional light position
        glm::mat4 lightView        = glm::lookAt(lightPos, glm::vec3(0, 0, 0), glm::vec3(0.0f, 1.0f, 0.0f));
        glm::mat4 lightSpaceMatrix = lightProjection * lightView;

        // for each terrain in the scene, request draw call.
        registry.view<TransformComponent, TerrainComponent, TerrainRendererComponent>().each(
            [this, pipeline, ownerPass, mainCameraTransform, mainCamera, directionalLight, lightSpaceMatrix](
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
                m_Shader->SetFloat3("viewPos", mainCameraTransform.Position);
                m_Shader->SetFloat3("directionalLight.direction", directionalLight.Direction);
                m_Shader->SetFloat("directionalLight.intensity", directionalLight.Intensity);
                m_Shader->SetFloat3("directionalLight.color", directionalLight.Color);

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

                // Bind shadow map
                m_Shader->SetMat4("lightSpaceMatrix", lightSpaceMatrix);
                ownerPass->GetShadowDepthBuffer()->BindDepthAttachmentTexture(1);
                m_Shader->SetInt("shadowMap", 1);
                m_Shader->SetInt("castShadow", terrainRenderer.CastShadow);

                // Currently, no static batching. leave temp test code here
                auto vertexArray = pipeline->GetAPI()->CreateVertexArray(terrain.Mesh);
                vertexArray->Bind();

                pipeline->GetAPI()->DrawIndexed(terrain.Mesh.Data.Indices.size());

                vertexArray->Unbind();
                m_Shader->Unbind();
            });

        if (rt != nullptr)
        {
            rt->Unbind();
        }
    }
} // namespace SnowLeopardEngine