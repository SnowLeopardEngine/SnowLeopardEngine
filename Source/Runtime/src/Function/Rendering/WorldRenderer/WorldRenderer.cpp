#include "SnowLeopardEngine/Function/Rendering/WorldRenderer/WorldRenderer.h"
#include "SnowLeopardEngine/Core/Base/Base.h"
#include "SnowLeopardEngine/Core/Base/Hash.h"
#include "SnowLeopardEngine/Core/Math/Frustum.h"
#include "SnowLeopardEngine/Core/Time/Time.h"
#include "SnowLeopardEngine/Engine/EngineContext.h"
#include "SnowLeopardEngine/Function/IO/TextureLoader.h"
#include "SnowLeopardEngine/Function/Rendering/Converters/CubemapConverter.h"
#include "SnowLeopardEngine/Function/Rendering/FrameGraph/TransientResources.h"
#include "SnowLeopardEngine/Function/Rendering/FrameUniform.h"
#include "SnowLeopardEngine/Function/Rendering/IndexBuffer.h"
#include "SnowLeopardEngine/Function/Rendering/LightUniform.h"
#include "SnowLeopardEngine/Function/Rendering/Renderable.h"
#include "SnowLeopardEngine/Function/Rendering/WorldRenderer/Passes/BlitUIPass.h"
#include "SnowLeopardEngine/Function/Rendering/WorldRenderer/Passes/DeferredLightingPass.h"
#include "SnowLeopardEngine/Function/Rendering/WorldRenderer/Passes/GaussianBlurPass.h"
#include "SnowLeopardEngine/Function/Rendering/WorldRenderer/Passes/InGameGUIPass.h"
#include "SnowLeopardEngine/Function/Rendering/WorldRenderer/Passes/SSAOPass.h"
#include "SnowLeopardEngine/Function/Rendering/WorldRenderer/Passes/ShadowPrePass.h"
#include "SnowLeopardEngine/Function/Rendering/WorldRenderer/Passes/SkyboxPass.h"
#include "SnowLeopardEngine/Function/Rendering/WorldRenderer/Passes/ToneMappingPass.h"
#include "SnowLeopardEngine/Function/Rendering/WorldRenderer/Resources/InGameGUIData.h"
#include "SnowLeopardEngine/Function/Rendering/WorldRenderer/Resources/SSAOData.h"
#include "SnowLeopardEngine/Function/Rendering/WorldRenderer/Resources/SceneColorData.h"
#include "SnowLeopardEngine/Function/Scene/Components.h"
#include "SnowLeopardEngine/Function/Scene/LogicScene.h"

#include <fg/Blackboard.hpp>
#include <fg/FrameGraph.hpp>

namespace SnowLeopardEngine
{
    void WorldRenderer::Init()
    {
        m_RenderContext      = CreateScope<RenderContext>();
        m_TransientResources = CreateScope<TransientResources>(*m_RenderContext);

        m_Viewport = m_RenderContext->GetViewport();

        CreateTools();
        CreatePasses();
    }

    void WorldRenderer::Shutdown() { m_RenderContext->Destroy(m_Skybox); }

    void WorldRenderer::OnLogicSceneLoaded(LogicScene* scene)
    {
        auto& registry = scene->GetRegistry();

        // Camera
        Entity mainCamera          = {registry.view<TransformComponent, CameraComponent>().front(), scene};
        auto&  mainCameraComponent = mainCamera.GetComponent<CameraComponent>();

        // TODO: Use texture path defined in camera
        assert(mainCameraComponent.IsEnvironmentMapHDR);
        auto* equirectangular = IO::Load(mainCameraComponent.EnvironmentMapFilePath, *m_RenderContext);
        m_Skybox              = m_CubemapConverter->EquirectangularToCubemap(*equirectangular);
        m_RenderContext->Destroy(*equirectangular);
    }

    void WorldRenderer::RenderFrame(float deltaTime)
    {
        FrameGraph           fg;
        FrameGraphBlackboard blackboard;

        CookRenderableScene();

        if (!m_MainCamera)
        {
            return;
        }

        auto defaultRenderables = FilterRenderables(m_Renderables, isDefault);

        // Update & upload frame uniform
        UpdateFrameUniform();
        uploadFrameUniform(fg, blackboard, m_FrameUniform);

        // Update & upload light uniform
        UpdateLightUniform(GetRenderablesAABB(defaultRenderables));
        uploadLightUniform(fg, blackboard, m_LightUniform);

        // Filter visable renderables (CPU Frustum culling)
        auto visableRenderables =
            FilterVisableRenderables(m_Renderables, m_FrameUniform.ProjectionMatrix * m_FrameUniform.ViewMatrix);

        // Shadow pre pass
        m_ShadowPrePass->AddToGraph(fg, blackboard, {.Width = 2048, .Height = 2048}, defaultRenderables);

        // G-Buffer pass
        auto opaqueRenderables        = FilterRenderables(visableRenderables, isOpaque);
        auto defaultOpaqueRenderables = FilterRenderables(opaqueRenderables, isDefault);
        auto groups                   = FilterRenderableGroups(defaultOpaqueRenderables);

        m_GBufferPass->AddToGraph(fg, blackboard, m_Viewport.Extent, groups);

        // SSAO pass
        m_SSAOPass->AddToGraph(fg, blackboard);
        auto& ssao = blackboard.get<SSAOData>().SSAO;
        ssao       = m_GaussianBlurPass->AddToGraph(fg, ssao, 1.0f);

        // Deferred lighting pass
        auto& sceneColor = blackboard.add<SceneColorData>();
        sceneColor.HDR   = m_DeferredLightingPass->AddToGraph(fg, blackboard);

        // Skybox pass
        sceneColor.HDR = m_SkyboxPass->AddToGraph(fg, blackboard, sceneColor.HDR, &m_Skybox);

        // Tone-Mapping pass
        sceneColor.LDR = m_ToneMappingPass->AddToGraph(fg, sceneColor.HDR);

        // FXAA pass
        sceneColor.LDR = m_FXAAPass->AddToGraph(fg, sceneColor.LDR);

        // In-Game GUI pass
        auto uiRenderables = FilterRenderables(m_Renderables, isUI);
        m_InGameGUIPass->AddToGraph(fg, blackboard, m_Viewport.Extent, uiRenderables);

        // Blit pass
        auto& [uiTarget, _] = blackboard.get<InGameGUIData>();
        sceneColor.LDR      = m_BlitUIPass->AddToGraph(fg, sceneColor.LDR, uiTarget);

        // Final composition
        m_FinalPass->Compose(fg, blackboard);

        fg.compile();
        fg.execute(m_RenderContext.get(), m_TransientResources.get());

#ifndef NDEBUG
        // Built in graphviz writer.
        std::ofstream {"DebugFrameGraph.dot"} << fg;
#endif

        m_TransientResources->Update(deltaTime);
    }

    void WorldRenderer::CreateTools()
    {
        auto& rc           = *m_RenderContext;
        m_CubemapConverter = CreateScope<CubemapConverter>(rc);
    }

    void WorldRenderer::CreatePasses()
    {
        auto& rc = *m_RenderContext;

        m_ShadowPrePass        = CreateScope<ShadowPrePass>(rc);
        m_GBufferPass          = CreateScope<GBufferPass>(rc);
        m_SSAOPass             = CreateScope<SSAOPass>(rc);
        m_GaussianBlurPass     = CreateScope<GaussianBlurPass>(rc);
        m_DeferredLightingPass = CreateScope<DeferredLightingPass>(rc);
        m_SkyboxPass           = CreateScope<SkyboxPass>(rc);
        m_ToneMappingPass      = CreateScope<ToneMappingPass>(rc);
        m_FXAAPass             = CreateScope<FXAAPass>(rc);
        m_InGameGUIPass        = CreateScope<InGameGUIPass>(rc);
        m_BlitUIPass           = CreateScope<BlitUIPass>(rc);
        m_FinalPass            = CreateScope<FinalPass>(rc);
    }

    void WorldRenderer::CookRenderableScene()
    {
        auto  scene    = g_EngineContext->SceneMngr->GetActiveScene();
        auto& registry = scene->GetRegistry();

        // Camera
        m_MainCamera = {registry.view<TransformComponent, CameraComponent>().front(), scene.get()};

        // Lights
        m_DirectionalLight = {registry.view<TransformComponent, DirectionalLightComponent>().front(), scene.get()};

        m_PointLights.clear();
        registry.view<TransformComponent, PointLightComponent>().each(
            [this, &scene](entt::entity e, TransformComponent&, PointLightComponent&) {
                Entity pointLight = {e, scene.get()};
                m_PointLights.emplace_back(pointLight);
            });

        // Collect renderables
        m_Renderables.clear();

        // Basic shapes (MeshFilter + MeshRenderer)
        registry.view<TransformComponent, MeshFilterComponent, MeshRendererComponent>().each(
            [this](entt::entity           e,
                   TransformComponent&    transform,
                   MeshFilterComponent&   meshFilter,
                   MeshRendererComponent& meshRenderer) {
                for (auto& mesh : meshFilter.Meshes->Items)
                {
                    if (!mesh.RenderLoaded)
                    {
                        // Load index buffer & vertex buffer
                        auto indexBuffer = m_RenderContext->CreateIndexBuffer(
                            IndexType::UInt32, mesh.Data.Indices.size(), mesh.Data.Indices.data());
                        auto vertexBuffer = m_RenderContext->CreateVertexBuffer(
                            mesh.Data.VertFormat->GetStride(), mesh.Data.Vertices.size(), mesh.Data.Vertices.data());

                        mesh.Data.IdxBuffer  = Ref<IndexBuffer>(new IndexBuffer {std::move(indexBuffer)},
                                                               RenderContext::ResourceDeleter {*m_RenderContext});
                        mesh.Data.VertBuffer = Ref<VertexBuffer>(new VertexBuffer {std::move(vertexBuffer)},
                                                                 RenderContext::ResourceDeleter {*m_RenderContext});
                        mesh.RenderLoaded    = true;
                    }

                    auto modelMatrix = transform.GetTransform();

                    Renderable renderable  = {};
                    renderable.Mesh        = &mesh;
                    renderable.Mat         = meshRenderer.Mat;
                    renderable.ModelMatrix = modelMatrix;
                    renderable.BoundingBox = AABB::Build(mesh.Data.Vertices).Transform(modelMatrix);
                    m_Renderables.emplace_back(renderable);
                }
            });

        // Terrains
        registry.view<TransformComponent, TerrainComponent, TerrainRendererComponent>().each(
            [this](entt::entity              e,
                   TransformComponent&       transform,
                   TerrainComponent&         terrain,
                   TerrainRendererComponent& terrainRenderer) {
                if (!terrain.Mesh.RenderLoaded)
                {
                    // Load index buffer & vertex buffer
                    auto indexBuffer = m_RenderContext->CreateIndexBuffer(
                        IndexType::UInt32, terrain.Mesh.Data.Indices.size(), terrain.Mesh.Data.Indices.data());
                    auto vertexBuffer = m_RenderContext->CreateVertexBuffer(terrain.Mesh.Data.VertFormat->GetStride(),
                                                                            terrain.Mesh.Data.Vertices.size(),
                                                                            terrain.Mesh.Data.Vertices.data());

                    terrain.Mesh.Data.IdxBuffer  = Ref<IndexBuffer>(new IndexBuffer {std::move(indexBuffer)},
                                                                   RenderContext::ResourceDeleter {*m_RenderContext});
                    terrain.Mesh.Data.VertBuffer = Ref<VertexBuffer>(new VertexBuffer {std::move(vertexBuffer)},
                                                                     RenderContext::ResourceDeleter {*m_RenderContext});
                    terrain.Mesh.RenderLoaded    = true;
                }

                auto modelMatrix = transform.GetTransform();

                Renderable renderable  = {};
                renderable.Mesh        = &terrain.Mesh;
                renderable.Mat         = terrainRenderer.Mat;
                renderable.ModelMatrix = modelMatrix;
                renderable.BoundingBox = AABB::Build(terrain.Mesh.Data.Vertices).Transform(modelMatrix);
                m_Renderables.emplace_back(renderable);
            });

        // UI
        // Image
        registry.view<UI::RectTransformComponent, UI::ImageComponent>().each([this](entt::entity                e,
                                                                                    UI::RectTransformComponent& rect,
                                                                                    UI::ImageComponent&         image) {
            if (!image.ImageMesh.RenderLoaded)
            {
                // Load index buffer & vertex buffer
                auto indexBuffer = m_RenderContext->CreateIndexBuffer(
                    IndexType::UInt32, image.ImageMesh.Data.Indices.size(), image.ImageMesh.Data.Indices.data());
                auto vertexBuffer = m_RenderContext->CreateVertexBuffer(image.ImageMesh.Data.VertFormat->GetStride(),
                                                                        image.ImageMesh.Data.Vertices.size(),
                                                                        image.ImageMesh.Data.Vertices.data());

                image.ImageMesh.Data.IdxBuffer  = Ref<IndexBuffer>(new IndexBuffer {std::move(indexBuffer)},
                                                                  RenderContext::ResourceDeleter {*m_RenderContext});
                image.ImageMesh.Data.VertBuffer = Ref<VertexBuffer>(new VertexBuffer {std::move(vertexBuffer)},
                                                                    RenderContext::ResourceDeleter {*m_RenderContext});
                image.ImageMesh.RenderLoaded    = true;
            }

            // set model matrix
            glm::mat4 model = glm::mat4(1.0f);
            model           = glm::translate(model, rect.Pos);
            model = glm::translate(model, glm::vec3(-rect.Pivot.x * rect.Size.x, -rect.Pivot.y * rect.Size.y, 0.0f));
            model = glm::rotate(model, rect.RotationAngle, glm::vec3(0.0f, 0.0f, 1.0f));
            model = glm::translate(model, glm::vec3(rect.Pivot.x * rect.Size.x, rect.Pivot.y * rect.Size.y, 0.0f));
            model = glm::translate(model, glm::vec3(-rect.Pivot.x * rect.Size.x, -rect.Pivot.y * rect.Size.y, 0.0f));
            model = glm::scale(model, glm::vec3(rect.Size, 1.0f));

            Renderable renderable           = {};
            renderable.Mesh                 = &image.ImageMesh;
            renderable.Mat                  = image.Mat;
            renderable.ModelMatrix          = model;
            renderable.BoundingBox          = AABB::Build(image.ImageMesh.Data.Vertices).Transform(model);
            renderable.Type                 = RenderableType::UI;
            renderable.UISpecs.ImageTexture = image.TargetGraphic;
            renderable.UISpecs.ImageColor   = image.Color;
            m_Renderables.emplace_back(renderable);
        });

        // Button
        registry.view<UI::RectTransformComponent, UI::ButtonComponent>().each([this](entt::entity                e,
                                                                                     UI::RectTransformComponent& rect,
                                                                                     UI::ButtonComponent& button) {
            if (!button.ImageMesh.RenderLoaded)
            {
                // Load index buffer & vertex buffer
                auto indexBuffer = m_RenderContext->CreateIndexBuffer(
                    IndexType::UInt32, button.ImageMesh.Data.Indices.size(), button.ImageMesh.Data.Indices.data());
                auto vertexBuffer = m_RenderContext->CreateVertexBuffer(button.ImageMesh.Data.VertFormat->GetStride(),
                                                                        button.ImageMesh.Data.Vertices.size(),
                                                                        button.ImageMesh.Data.Vertices.data());

                button.ImageMesh.Data.IdxBuffer  = Ref<IndexBuffer>(new IndexBuffer {std::move(indexBuffer)},
                                                                   RenderContext::ResourceDeleter {*m_RenderContext});
                button.ImageMesh.Data.VertBuffer = Ref<VertexBuffer>(new VertexBuffer {std::move(vertexBuffer)},
                                                                     RenderContext::ResourceDeleter {*m_RenderContext});
                button.ImageMesh.RenderLoaded    = true;
            }

            // set model matrix
            glm::mat4 model = glm::mat4(1.0f);
            model           = glm::translate(model, rect.Pos);
            model = glm::translate(model, glm::vec3(-rect.Pivot.x * rect.Size.x, -rect.Pivot.y * rect.Size.y, 0.0f));
            model = glm::rotate(model, rect.RotationAngle, glm::vec3(0.0f, 0.0f, 1.0f));
            model = glm::translate(model, glm::vec3(rect.Pivot.x * rect.Size.x, rect.Pivot.y * rect.Size.y, 0.0f));
            model = glm::translate(model, glm::vec3(-rect.Pivot.x * rect.Size.x, -rect.Pivot.y * rect.Size.y, 0.0f));
            model = glm::scale(model, glm::vec3(rect.Size, 1.0f));

            Renderable renderable                     = {};
            renderable.Mesh                           = &button.ImageMesh;
            renderable.Mat                            = button.Mat;
            renderable.ModelMatrix                    = model;
            renderable.BoundingBox                    = AABB::Build(button.ImageMesh.Data.Vertices).Transform(model);
            renderable.Type                           = RenderableType::UI;
            renderable.UISpecs.ButtonColorTintTexture = button.TintColor.TargetGraphic;
            renderable.UISpecs.ButtonColorTintCurrentColor = button.TintColor.Current;
            m_Renderables.emplace_back(renderable);
        });
    }

    void WorldRenderer::UpdateFrameUniform()
    {
        auto& mainCameraTransformComponent = m_MainCamera.GetComponent<TransformComponent>();
        auto& mainCameraComponent          = m_MainCamera.GetComponent<CameraComponent>();
        mainCameraComponent.ViewportWidth  = g_EngineContext->WindowSys->GetWidth();
        mainCameraComponent.ViewportHeight = g_EngineContext->WindowSys->GetHeight();

        m_FrameUniform.ElapsedTime = Time::ElapsedTime;

        m_FrameUniform.Resolution = {m_Viewport.Extent.Width, m_Viewport.Extent.Height};

        m_FrameUniform.ViewPos            = mainCameraTransformComponent.Position;
        m_FrameUniform.ViewMatrix         = g_EngineContext->CameraSys->GetViewMatrix(mainCameraTransformComponent);
        m_FrameUniform.InversedViewMatrix = glm::inverse(m_FrameUniform.ViewMatrix);
        m_FrameUniform.ProjectionMatrix   = g_EngineContext->CameraSys->GetProjectionMatrix(mainCameraComponent);
        m_FrameUniform.InversedProjectionMatrix = glm::inverse(m_FrameUniform.ProjectionMatrix);

        CameraComponent tempOrtho            = mainCameraComponent;
        tempOrtho.Projection                 = CameraProjection::Orthographic;
        m_FrameUniform.OrthoProjectionMatrix = g_EngineContext->CameraSys->GetProjectionMatrix(tempOrtho);
    }

    void WorldRenderer::UpdateLightUniform(const AABB& renderablesAABB)
    {
        auto& directionalLightComponent = m_DirectionalLight.GetComponent<DirectionalLightComponent>();

        GPUDirectionalLight directionalLight {};
        directionalLight.Direction = directionalLightComponent.Direction;
        directionalLight.Color     = directionalLightComponent.Color;
        directionalLight.Intensity = directionalLightComponent.Intensity;

        // Get renderables AABB and set light space.
        auto sceneAABBCenter = renderablesAABB.GetCenter();
        auto sceneAABBSize   = renderablesAABB.GetExtent();

        float     lightDistance = 2.0f * glm::length(sceneAABBSize);
        glm::vec3 lightPos      = sceneAABBCenter - lightDistance * directionalLightComponent.Direction;

        glm::mat4 lightProjection = glm::ortho(
            -sceneAABBSize.x, sceneAABBSize.x, -sceneAABBSize.y, sceneAABBSize.y, 0.0f, 2.0f * lightDistance);
        glm::mat4 lightView = glm::lookAt(lightPos, sceneAABBCenter, glm::vec3(0.0f, 1.0f, 0.0f));

        glm::mat4 lightSpaceMatrix = lightProjection * lightView;

        directionalLight.LightSpaceMatrix = lightSpaceMatrix;

        m_LightUniform.DirectionalLight = directionalLight;

        uint32_t pointLightIndex = 0;
        for (auto& pointLight : m_PointLights)
        {
            if (pointLightIndex >= kMaxPointLights)
            {
                break;
            }

            auto& pointLightComponent = pointLight.GetComponent<PointLightComponent>();

            GPUPointLight gpuPointLight {};

            gpuPointLight.Constant  = pointLightComponent.Constant;
            gpuPointLight.Linear    = pointLightComponent.Linear;
            gpuPointLight.Quadratic = pointLightComponent.Quadratic;
            gpuPointLight.Intensity = pointLightComponent.Intensity;
            gpuPointLight.Position  = pointLight.GetComponent<TransformComponent>().Position;
            gpuPointLight.Color     = pointLightComponent.Color;

            float     nearPlane        = 0.1f;
            float     farPlane         = 100.0f;
            glm::mat4 lightProjection  = glm::perspective(glm::radians(90.0f), 1.0f, nearPlane, farPlane);
            glm::mat4 lightView        = glm::lookAt(gpuPointLight.Position,
                                              gpuPointLight.Position + glm::vec3(1.0f, 0.0f, 0.0f),
                                              glm::vec3(0.0f, 1.0f, 0.0f));
            glm::mat4 lightSpaceMatrix = lightProjection * lightView;

            gpuPointLight.LightSpaceMatrix = lightSpaceMatrix;

            m_LightUniform.PointLights[pointLightIndex] = gpuPointLight;

            pointLightIndex++;
        }
        m_LightUniform.NumPointLights = pointLightIndex;
    }

    std::vector<Renderable> WorldRenderer::FilterVisableRenderables(std::span<Renderable> renderables,
                                                                    const glm::mat4&      cameraViewProjection)
    {
        Frustum cameraFrustum {};
        cameraFrustum.Update(cameraViewProjection);

        std::vector<Renderable> visableRenderables;
        for (const auto& renderable : renderables)
        {
            if (cameraFrustum.TestAABB(renderable.BoundingBox))
            {
                visableRenderables.emplace_back(renderable);
            }
        }

        return visableRenderables;
    }

    std::vector<Renderable> WorldRenderer::FilterRenderables(std::span<Renderable> src, auto&& predicate)
    {
        auto out = src | std::views::filter(predicate);
        return {out.begin(), out.end()};
    }

    RenderableGroups WorldRenderer::FilterRenderableGroups(std::span<Renderable> renderables)
    {
        RenderableGroups groups;

        for (auto& renderable : renderables)
        {
            auto hash = renderable.Mesh->Data.VertFormat->GetHash();
            if (renderable.Mat)
                hashCombine(hash, renderable.Mat->GetHash());

            if (groups.count(hash) == 0)
            {
                groups[hash] = {};
                if (renderable.Mat->GetDefine().EnableInstancing)
                {
                    groups[hash].GroupType = RenderableGroupType::Instancing;
                }
            }

            groups[hash].Renderables.emplace_back(renderable);
        }

        return groups;
    }

    AABB WorldRenderer::GetRenderablesAABB(std::span<Renderable> renderables)
    {
        float maxFloat        = std::numeric_limits<float>().max();
        AABB  renderablesAABB = {.Min = glm::vec3(maxFloat), .Max = glm::vec3(-maxFloat)};

        for (const auto& renderable : renderables)
        {
            renderablesAABB.Merge(renderable.BoundingBox);
        }

        return renderablesAABB;
    }
} // namespace SnowLeopardEngine