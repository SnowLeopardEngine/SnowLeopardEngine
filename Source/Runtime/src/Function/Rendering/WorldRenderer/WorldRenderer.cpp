#include "SnowLeopardEngine/Function/Rendering/WorldRenderer/WorldRenderer.h"
#include "SnowLeopardEngine/Core/Base/Base.h"
#include "SnowLeopardEngine/Core/Math/Frustum.h"
#include "SnowLeopardEngine/Core/Time/Time.h"
#include "SnowLeopardEngine/Engine/EngineContext.h"
#include "SnowLeopardEngine/Function/Rendering/FrameGraph/TransientResources.h"
#include "SnowLeopardEngine/Function/Rendering/FrameUniform.h"
#include "SnowLeopardEngine/Function/Rendering/IndexBuffer.h"
#include "SnowLeopardEngine/Function/Rendering/Renderable.h"
#include "SnowLeopardEngine/Function/Rendering/WorldRenderer/Passes/DeferredLightingPass.h"
#include "SnowLeopardEngine/Function/Rendering/WorldRenderer/Passes/ShadowPrePass.h"
#include "SnowLeopardEngine/Function/Rendering/WorldRenderer/Resources/SceneColorData.h"
#include "SnowLeopardEngine/Function/Scene/Components.h"
#include "SnowLeopardEngine/Function/Scene/LogicScene.h"

#include <fg/Blackboard.hpp>

namespace SnowLeopardEngine
{
    void WorldRenderer::Init()
    {
        m_RenderContext      = CreateScope<RenderContext>();
        m_TransientResources = CreateScope<TransientResources>(*m_RenderContext);

        CreatePasses();
    }

    void WorldRenderer::OnLogicSceneLoaded(LogicScene* scene) {}

    void WorldRenderer::CookRenderableScene()
    {
        auto  scene    = g_EngineContext->SceneMngr->GetActiveScene();
        auto& registry = scene->GetRegistry();

        // Camera
        m_MainCamera = {registry.view<TransformComponent, CameraComponent>().front(), scene.get()};

        // Lights
        m_DirectionalLight = {registry.view<TransformComponent, DirectionalLightComponent>().front(), scene.get()};

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
                    // Load index buffer & vertex buffer
                    auto indexBuffer = m_RenderContext->CreateIndexBuffer(
                        IndexType::UInt32, mesh.Data.Indices.size(), mesh.Data.Indices.data());
                    auto vertexBuffer = m_RenderContext->CreateVertexBuffer(
                        mesh.Data.VertFormat->GetStride(), mesh.Data.Vertices.size(), mesh.Data.Vertices.data());

                    mesh.Data.IdxBuffer  = Ref<IndexBuffer>(new IndexBuffer {std::move(indexBuffer)},
                                                           RenderContext::ResourceDeleter {*m_RenderContext});
                    mesh.Data.VertBuffer = Ref<VertexBuffer>(new VertexBuffer {std::move(vertexBuffer)},
                                                             RenderContext::ResourceDeleter {*m_RenderContext});

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

                auto modelMatrix = transform.GetTransform();

                Renderable renderable  = {};
                renderable.Mesh        = &terrain.Mesh;
                renderable.Mat         = terrainRenderer.Mat;
                renderable.ModelMatrix = modelMatrix;
                renderable.BoundingBox = AABB::Build(terrain.Mesh.Data.Vertices).Transform(modelMatrix);
                m_Renderables.emplace_back(renderable);
            });
    }

    void WorldRenderer::RenderFrame(float deltaTime)
    {
        FrameGraph           fg;
        FrameGraphBlackboard blackboard;

        CookRenderableScene();

        if (!m_MainCamera || !m_DirectionalLight)
        {
            return;
        }

        // Setup camera properties
        auto& mainCameraTransformComponent = m_MainCamera.GetComponent<TransformComponent>();
        auto& mainCameraComponent          = m_MainCamera.GetComponent<CameraComponent>();
        mainCameraComponent.ViewportWidth  = g_EngineContext->WindowSys->GetWidth();
        mainCameraComponent.ViewportHeight = g_EngineContext->WindowSys->GetHeight();

        auto& directionalLightComponent = m_DirectionalLight.GetComponent<DirectionalLightComponent>();

        auto viewPort = m_RenderContext->GetViewport();

        // Update & upload frame uniform
        m_FrameUniform.ElapsedTime           = Time::ElapsedTime;
        m_FrameUniform.ViewPos               = mainCameraTransformComponent.Position;
        m_FrameUniform.ViewMatrix            = g_EngineContext->CameraSys->GetViewMatrix(mainCameraTransformComponent);
        m_FrameUniform.ProjectionMatrix      = g_EngineContext->CameraSys->GetProjectionMatrix(mainCameraComponent);
        m_FrameUniform.DirectionalLightColor = directionalLightComponent.Color;
        m_FrameUniform.DirectionalLightDirection = directionalLightComponent.Direction;
        m_FrameUniform.DirectionalLightIntensity = directionalLightComponent.Intensity;

        // Filter visable renderables (CPU Frustum culling)
        auto visableRenderables =
            FilterVisableRenderables(m_Renderables, m_FrameUniform.ProjectionMatrix * m_FrameUniform.ViewMatrix);

        // Get visable AABB and set light space.
        auto visableAABB       = GetVisableAABB(visableRenderables);
        auto visableAABBCenter = visableAABB.GetCenter();
        auto visableAABBSize   = visableAABB.GetExtent();

        float     lightDistance = 2.0f * glm::length(visableAABBSize);
        glm::vec3 lightPos      = visableAABBCenter - lightDistance * directionalLightComponent.Direction;

        glm::mat4 lightProjection = glm::ortho(
            -visableAABBSize.x, visableAABBSize.x, -visableAABBSize.y, visableAABBSize.y, 0.0f, 2.0f * lightDistance);
        glm::mat4 lightView = glm::lookAt(lightPos, visableAABBCenter, glm::vec3(0.0f, 1.0f, 0.0f));

        glm::mat4 lightSpaceMatrix = lightProjection * lightView;

        m_FrameUniform.LightSpaceMatrix = lightSpaceMatrix;

        uploadFrameUniform(fg, blackboard, m_FrameUniform);

        // Shadow pre pass
        m_ShadowPrePass->AddToGraph(fg, blackboard, {.Width = 2048, .Height = 2048}, visableRenderables);

        // G-Buffer pass
        m_GBufferPass->AddToGraph(fg, blackboard, viewPort.Extent, visableRenderables);

        // Deferred lighting pass
        auto& sceneColor = blackboard.add<SceneColorData>();
        sceneColor.HDR = m_DeferredLightingPass->AddToGraph(fg, blackboard);

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

    void WorldRenderer::CreatePasses()
    {
        m_ShadowPrePass        = CreateScope<ShadowPrePass>(*m_RenderContext);
        m_GBufferPass          = CreateScope<GBufferPass>(*m_RenderContext);
        m_DeferredLightingPass = CreateScope<DeferredLightingPass>(*m_RenderContext);
        m_FinalPass            = CreateScope<FinalPass>(*m_RenderContext);
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

    AABB WorldRenderer::GetVisableAABB(std::span<Renderable> visableRenderables)
    {
        float maxFloat    = std::numeric_limits<float>().max();
        AABB  visableAABB = {.Min = glm::vec3(maxFloat), .Max = glm::vec3(-maxFloat)};

        for (const auto& renderable : visableRenderables)
        {
            visableAABB.Merge(renderable.BoundingBox);
        }

        return visableAABB;
    }

    AABB WorldRenderer::GetRenderableSceneAABB()
    {
        float maxFloat  = std::numeric_limits<float>().max();
        AABB  sceneAABB = {.Min = glm::vec3(maxFloat), .Max = glm::vec3(-maxFloat)};

        for (const auto& renderable : m_Renderables)
        {
            sceneAABB.Merge(renderable.BoundingBox);
        }

        return sceneAABB;
    }
} // namespace SnowLeopardEngine