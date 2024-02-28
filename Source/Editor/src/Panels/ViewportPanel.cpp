#include "SnowLeopardEditor/Panels/ViewportPanel.h"
#include "SnowLeopardEditor/Selector.h"
#include "SnowLeopardEngine/Engine/EngineContext.h"
#include "SnowLeopardEngine/Function/Input/Input.h"
#include "SnowLeopardEngine/Function/Scene/Components.h"

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
        m_EditorCamera                                             = scene->CreateEntity("MainCamera");
        m_EditorCamera.GetComponent<TransformComponent>().Position = {0, 10, 30};
        auto& cameraComponent                                      = m_EditorCamera.AddComponent<CameraComponent>();
        // TODO: Add editor camera controller script
        cameraComponent.ClearFlags = CameraClearFlags::Skybox; // Enable skybox
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
        HandleInput();

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
            int entityID    = m_RenderTarget->ReadPixelRedOnly(1, mousePos.x, mousePos.y);
            m_HoveredEntity = entityID == -1 ? Entity() :
                                               Entity(static_cast<entt::entity>(entityID),
                                                      g_EngineContext->SceneMngr->GetActiveScene().get());
            m_RenderTarget->Unbind();
        }

        m_IsWindowHovered = ImGui::IsWindowHovered();

        // Gizmos
        auto selectedEntityUUID = Selector::GetLastSelection(SelectionCategory::Viewport);
        if (selectedEntityUUID.has_value())
        {
            Entity selectedEntity =
                g_EngineContext->SceneMngr->GetActiveScene()->GetEntityWithCoreUUID(selectedEntityUUID.value());
            if (selectedEntity)
            {
                ImGuizmo::SetOrthographic(false);
                ImGuizmo::SetDrawlist();

                ImGuizmo::SetRect(bounds0.x, bounds0.y, bounds1.x - bounds0.x, bounds1.y - bounds0.y);

                // Editor camera
                auto editorCameraComponent          = m_EditorCamera.GetComponent<CameraComponent>();
                auto editorCameraTransformComponent = m_EditorCamera.GetComponent<TransformComponent>();
                auto cameraView       = g_EngineContext->CameraSys->GetViewMatrix(editorCameraTransformComponent);
                auto cameraProjection = g_EngineContext->CameraSys->GetProjectionMatrix(editorCameraComponent);

                // Entity transform
                auto&     transformComponent = selectedEntity.GetComponent<TransformComponent>();
                glm::mat4 transform          = transformComponent.GetTransform();

                // Snapping
                bool snap = g_EngineContext->InputSys->GetKey(KeyCode::LeftControl);

                float snapValue = 0.5f; // Snap to 0.5m for translation/scale

                // Snap to 45 degrees for rotation
                if (m_GuizmoOperation == ImGuizmo::OPERATION::ROTATE)
                {
                    snapValue = 45.0f;
                }

                float snapValues[3] = {snapValue, snapValue, snapValue};

                ImGuizmo::Manipulate(glm::value_ptr(cameraView),
                                     glm::value_ptr(cameraProjection),
                                     m_GuizmoOperation,
                                     ImGuizmo::LOCAL,
                                     glm::value_ptr(transform),
                                     nullptr,
                                     snap ? snapValues : nullptr);

                if (ImGuizmo::IsUsing())
                {
                    glm::vec3 rotation;
                    ImGuizmo::DecomposeMatrixToComponents(glm::value_ptr(transform),
                                                          glm::value_ptr(transformComponent.Position),
                                                          glm::value_ptr(rotation),
                                                          glm::value_ptr(transformComponent.Scale));
                    transformComponent.SetRotationEuler(rotation);
                }
            }
        }

        ImGui::End();
    }

    void ViewportPanel::HandleInput()
    {
        // Select hovered entity
        if (g_EngineContext->InputSys->GetMouseButtonDown(MouseCode::ButtonLeft) && m_IsWindowHovered &&
            !ImGuizmo::IsOver())
        {
            if (m_HoveredEntity)
            {
                SNOW_LEOPARD_INFO("[Editor][Viewport] Selected entity: Name = {0}",
                                  m_HoveredEntity.GetComponent<NameComponent>().Name);

                // Single selection for now
                Selector::UnselectAll(SelectionCategory::Viewport);
                Selector::Select(SelectionCategory::Viewport, m_HoveredEntity.GetCoreUUID());
            }
            else
            {
                // https://github.com/CedricGuillemet/ImGuizmo/issues/133#issuecomment-708083755
                auto selectedEntityUUID = Selector::GetLastSelection(SelectionCategory::Viewport);
                if (selectedEntityUUID.has_value())
                {
                    auto entityCache =
                        g_EngineContext->SceneMngr->GetActiveScene()->GetEntityWithCoreUUID(selectedEntityUUID.value());

                    // Editor camera
                    auto editorCameraComponent          = m_EditorCamera.GetComponent<CameraComponent>();
                    auto editorCameraTransformComponent = m_EditorCamera.GetComponent<TransformComponent>();
                    auto cameraView       = g_EngineContext->CameraSys->GetViewMatrix(editorCameraTransformComponent);
                    auto cameraProjection = g_EngineContext->CameraSys->GetProjectionMatrix(editorCameraComponent);

                    glm::mat4 transform = entityCache.GetComponent<TransformComponent>().GetTransform();
                    transform           = glm::translate(transform, glm::vec3(0.0f, -10000.0f, 0.0f));
                    ImGuizmo::Manipulate(glm::value_ptr(cameraView),
                                         glm::value_ptr(cameraProjection),
                                         m_GuizmoOperation,
                                         m_GuizmoMode,
                                         glm::value_ptr(transform));
                }

                Selector::UnselectAll(SelectionCategory::Viewport);
            }
        }

        if (g_EngineContext->InputSys->GetKeyDown(KeyCode::W) && !ImGuizmo::IsUsing())
        {
            m_GuizmoOperation = ImGuizmo::OPERATION::TRANSLATE;
        }

        if (g_EngineContext->InputSys->GetKeyDown(KeyCode::E) && !ImGuizmo::IsUsing())
        {
            m_GuizmoOperation = ImGuizmo::OPERATION::ROTATE;
        }

        if (g_EngineContext->InputSys->GetKeyDown(KeyCode::R) && !ImGuizmo::IsUsing())
        {
            m_GuizmoOperation = ImGuizmo::OPERATION::SCALE;
        }
    }
} // namespace SnowLeopardEngine::Editor