#include "SnowLeopardEditor/Panels/ViewportPanel.h"
#include "SnowLeopardEngine/Engine/EngineContext.h"
#include "SnowLeopardEngine/Function/Scene/Components.h"
#include "entt/entity/fwd.hpp"
#include <SnowLeopardEngine/Function/Scene/Entity.h>

#include <imgui.h>

namespace SnowLeopardEngine::Editor
{
    void ViewportPanel::Init()
    {
        // Create RT

        // Color Attachment 0 - main target color
        FrameBufferTextureDesc rtColorAttachment0Desc = {};
        rtColorAttachment0Desc.TextureFormat          = FrameBufferTextureFormat::RGBA8;

        // Color Attachment 1 - picking objects, back buffer hack
        FrameBufferTextureDesc rtColorAttachment1Desc = {};
        rtColorAttachment1Desc.TextureFormat          = FrameBufferTextureFormat::RED_INTEGER;

        // Depth Attachment
        FrameBufferTextureDesc rtDepthAttachmentDesc;
        rtDepthAttachmentDesc.TextureFormat = FrameBufferTextureFormat::DEPTH24_STENCIL8;

        FrameBufferAttachmentDesc rtAttachmentDesc = {};
        rtAttachmentDesc.Attachments.emplace_back(rtColorAttachment0Desc);
        rtAttachmentDesc.Attachments.emplace_back(rtColorAttachment1Desc);
        rtAttachmentDesc.Attachments.emplace_back(rtDepthAttachmentDesc);

        FrameBufferDesc rtDesc = {};
        rtDesc.Width           = m_ViewportSize.x;
        rtDesc.Height          = m_ViewportSize.y;
        rtDesc.AttachmentDesc  = rtAttachmentDesc;

        m_RenderTarget = FrameBuffer::Create(rtDesc);

        // Set RT
        g_EngineContext->RenderSys->GetPipeline()->SetRenderTarget(m_RenderTarget);

        // TODO: remove , test only code
        // Create a scene and set active
        auto scene = g_EngineContext->SceneMngr->CreateScene("RenderingSystem", true);

        // Create a camera
        Entity camera                                      = scene->CreateEntity("MainCamera");
        camera.GetComponent<TransformComponent>().Position = {0, 10, 30};
        auto& cameraComponent                              = camera.AddComponent<CameraComponent>();
        cameraComponent.ClearFlags                         = CameraClearFlags::Skybox; // Enable skybox
        // clang-format off
        cameraComponent.CubemapFilePaths = {
            "Assets/Textures/Skybox001/right.jpg",
            "Assets/Textures/Skybox001/left.jpg",
            "Assets/Textures/Skybox001/top.jpg",
            "Assets/Textures/Skybox001/bottom.jpg",
            "Assets/Textures/Skybox001/front.jpg",
            "Assets/Textures/Skybox001/back.jpg"
        };
        // clang-format on

        // Create cubes to test shadow
        Entity cube1            = scene->CreateEntity("Cube1");
        auto&  cubeTransform1   = cube1.GetComponent<TransformComponent>();
        cubeTransform1.Position = {0, 10, 0};
        cubeTransform1.SetRotationEuler(glm::vec3(45, 45, 45));
        auto& cubeMeshFilter1         = cube1.AddComponent<MeshFilterComponent>();
        cubeMeshFilter1.PrimitiveType = MeshPrimitiveType::Cube;
        auto& cubeMeshRenderer1       = cube1.AddComponent<MeshRendererComponent>();
        cubeMeshRenderer1.BaseColor   = {1, 0, 0, 1};

        Entity cube2            = scene->CreateEntity("Cube2");
        auto&  cubeTransform2   = cube2.GetComponent<TransformComponent>();
        cubeTransform2.Position = {-3, 5, -5};
        cubeTransform2.SetRotationEuler(glm::vec3(135, 135, 135));
        cubeTransform2.Scale          = {2, 2, 2};
        auto& cubeMeshFilter2         = cube2.AddComponent<MeshFilterComponent>();
        cubeMeshFilter2.PrimitiveType = MeshPrimitiveType::Cube;
        auto& cubeMeshRenderer2       = cube2.AddComponent<MeshRendererComponent>();
        cubeMeshRenderer2.BaseColor   = {0, 1, 0, 1};

        // Create a floor
        Entity floor = scene->CreateEntity("Floor");

        auto& floorTransform          = floor.GetComponent<TransformComponent>();
        floorTransform.Scale          = {50, 1, 50};
        auto& floorMeshFilter         = floor.AddComponent<MeshFilterComponent>();
        floorMeshFilter.PrimitiveType = MeshPrimitiveType::Cube;
        auto& floorMeshRenderer       = floor.AddComponent<MeshRendererComponent>();
        floorMeshRenderer.BaseColor   = {1, 1, 1, 1}; // Pure White
    }

    void ViewportPanel::OnFixedTick()
    {
        // Tick Logic
        g_EngineContext->SceneMngr->OnFixedTick();

        // Tick Physics
        g_EngineContext->PhysicsSys->OnFixedTick();
    }

    void ViewportPanel::OnTick(float deltaTime)
    {
        // Tick logic
        g_EngineContext->SceneMngr->OnTick(deltaTime);

        m_RenderTarget->Bind();
        m_RenderTarget->ClearColorAttachment(1, 0); // Clear RED buffer (picking buffer)
        m_RenderTarget->Unbind();

        if (!g_EngineContext->WindowSys->IsMinimized())
        {
            // Tick rendering system
            g_EngineContext->RenderSys->OnTick(deltaTime);
        }

        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
        ImGui::Begin("Viewport");
        ImGui::PopStyleVar();

        uint32_t colorAttachment0 = m_RenderTarget->GetColorAttachmentID(0);
        if (colorAttachment0)
        {
            // update viewport size & frame buffer size
            auto      size         = ImGui::GetContentRegionAvail();
            glm::vec2 viewportSize = {size.x, size.y};

            if (m_ViewportSize != viewportSize)
            {
                m_RenderTarget->Resize(viewportSize.x, viewportSize.y);
                m_ViewportSize = viewportSize;
            }

            // Render Framebuffer to an image.
            ImGui::Image(reinterpret_cast<ImTextureID>(static_cast<intptr_t>(colorAttachment0)), size, {0, 1}, {1, 0});
        }

        // Get mouse relative position (Origin is left bottom) to the viewport window
        auto      viewportMinRegion = ImGui::GetWindowContentRegionMin();
        auto      viewportMaxRegion = ImGui::GetWindowContentRegionMax();
        auto      viewportOffset    = ImGui::GetWindowPos();
        glm::vec2 bounds0           = {viewportMinRegion.x + viewportOffset.x, viewportMinRegion.y + viewportOffset.y};
        glm::vec2 bounds1           = {viewportMaxRegion.x + viewportOffset.x, viewportMaxRegion.y + viewportOffset.y};
        ImVec2    mousePos          = ImGui::GetMousePos();
        mousePos.x -= bounds0.x;
        mousePos.y -= bounds0.y;
        auto viewportTotalSize = bounds1 - bounds0;
        mousePos.y             = viewportTotalSize.y - mousePos.y;

        if (mousePos.x >= 0 && mousePos.x < viewportTotalSize.x && mousePos.y >= 0 && mousePos.y < viewportTotalSize.y)
        {
            m_RenderTarget->Bind();
            int entityID = m_RenderTarget->ReadPixelRedOnly(1, mousePos.x, mousePos.y);
            if (entityID != -1)
            {
                Entity hoveredEntity = {static_cast<entt::entity>(entityID),
                                        g_EngineContext->SceneMngr->GetActiveScene().get()};
                SNOW_LEOPARD_INFO("[Editor][Viewport] Hovered on entity: Name={0}",
                                  hoveredEntity.GetComponent<NameComponent>().Name);
            }
            m_RenderTarget->Unbind();
        }

        ImGui::End();
    }
} // namespace SnowLeopardEngine::Editor