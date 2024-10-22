#include "SnowLeopardEditor/Panels/ViewportPanel.h"
#include "SnowLeopardEditor/EditorCamera/EditorCameraScript.h"
#include "SnowLeopardEditor/Selector.h"
#include "SnowLeopardEngine/Core/Reflection/TypeFactory.h"
#include "SnowLeopardEngine/Engine/EngineContext.h"
#include "SnowLeopardEngine/Function/Asset/Model.h"
#include "SnowLeopardEngine/Function/Input/Input.h"
#include "SnowLeopardEngine/Function/Scene/Components.h"

#include "IconsMaterialDesignIcons.h"
#include "entt/entt.hpp"
#include <imgui.h>

namespace SnowLeopardEngine::Editor
{
    void ViewportPanel::Init(const PanelCommonInitInfo& initInfo)
    {
        REGISTER_TYPE(EditorCameraScript);

        // TODO: Set RT
    }

    void ViewportPanel::OnFixedTick()
    {
        // Tick Logic
        g_EngineContext->SceneMngr->OnFixedTick();

        if (m_ViewportMode == ViewportMode::Simulating)
        {
            // Tick Physics
            g_EngineContext->PhysicsSys->OnFixedTick();
        }
    }

    void ViewportPanel::OnTick(float deltaTime)
    {
        HandleInput();

        // Tick logic
        g_EngineContext->SceneMngr->OnTick(deltaTime);

        // m_RenderTarget->Bind();
        // m_RenderTarget->ClearColorAttachment(1, -1); // Clear RED buffer (picking buffer)
        // m_RenderTarget->Unbind();

        if (!g_EngineContext->WindowSys->IsMinimized())
        {
            // Tick rendering system
            g_EngineContext->RenderSys->OnTick(deltaTime);
        }

        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
        ImGui::Begin("Viewport");
        ImGui::PopStyleVar();

        ImGui::BeginChild("ViewportToolbar", ImVec2(0, 30), false, ImGuiWindowFlags_NoScrollbar);
        DrawToolbar();
        ImGui::EndChild();

        ImGui::BeginChild("ViewportMain", ImVec2(0, 0), false, ImGuiWindowFlags_NoScrollbar);
        // uint32_t colorAttachment0 = m_RenderTarget->GetColorAttachmentID(0);
        // if (colorAttachment0)
        // {
        //     // update viewport size & frame buffer size
        //     auto      size         = ImGui::GetContentRegionAvail();
        //     glm::vec2 viewportSize = {size.x, size.y};

        //     if (m_ViewportSize != viewportSize)
        //     {
        //         m_RenderTarget->Resize(viewportSize.x, viewportSize.y);
        //         m_ViewportSize = viewportSize;
        //     }

        //     // Render Framebuffer to an image.
        //     ImGui::Image(reinterpret_cast<ImTextureID>(static_cast<intptr_t>(colorAttachment0)), size, {0, 1}, {1, 0});
        // }

        if (m_ViewportMode == ViewportMode::Edit)
        {
            // Get mouse relative position (Origin is left bottom) to the viewport window
            auto      viewportMinRegion = ImGui::GetWindowContentRegionMin();
            auto      viewportMaxRegion = ImGui::GetWindowContentRegionMax();
            auto      viewportOffset    = ImGui::GetWindowPos();
            glm::vec2 bounds0  = {viewportMinRegion.x + viewportOffset.x, viewportMinRegion.y + viewportOffset.y};
            glm::vec2 bounds1  = {viewportMaxRegion.x + viewportOffset.x, viewportMaxRegion.y + viewportOffset.y};
            ImVec2    mousePos = ImGui::GetMousePos();
            mousePos.x -= bounds0.x;
            mousePos.y -= bounds0.y;
            auto viewportTotalSize = bounds1 - bounds0;
            mousePos.y             = viewportTotalSize.y - mousePos.y;

            if (mousePos.x >= 0 && mousePos.x < viewportTotalSize.x && mousePos.y >= 0 &&
                mousePos.y < viewportTotalSize.y)
            {
                // // Get picking buffer pixel value (entity ID)
                // m_RenderTarget->Bind();
                // int entityID = m_RenderTarget->ReadPixelRedOnly(1, mousePos.x, mousePos.y);
                // m_HoveredEntity =
                //     (entityID == -1 || m_GuizmoOperation == -1) ?
                //         Entity() :
                //         Entity(static_cast<entt::entity>(entityID), g_EngineContext->SceneMngr->GetActiveScene().get());
                // m_RenderTarget->Unbind();
            }

            m_IsWindowHovered = ImGui::IsWindowHovered();

            // Set EditorCamera states
            if (m_EditorCamera)
            {
                if (m_EditorCameraScript == nullptr)
                {
                    m_EditorCameraScript = std::dynamic_pointer_cast<EditorCameraScript>(
                        m_EditorCamera.GetComponent<NativeScriptingComponent>().ScriptInstance);
                }
                m_EditorCameraScript->SetWindowHovered(m_IsWindowHovered);
                m_EditorCameraScript->SetGrabMoveEnabled(m_GuizmoOperation == -1);
            }

            // Gizmos
            auto selectedEntityUUID = Selector::GetLastSelection(SelectionCategory::Viewport);
            if (selectedEntityUUID.has_value())
            {
                Entity selectedEntity =
                    g_EngineContext->SceneMngr->GetActiveScene()->GetEntityWithCoreUUID(selectedEntityUUID.value());
                if (selectedEntity && m_GuizmoOperation != -1)
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
                                         static_cast<ImGuizmo::OPERATION>(m_GuizmoOperation),
                                         m_GuizmoMode,
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
        }

        ImGui::EndChild();
        ImGui::End();
    }

    void ViewportPanel::HandleInput()
    {
        if (m_ViewportMode != ViewportMode::Edit)
        {
            return;
        }

        // Select hovered entity

        // Workaround for https://github.com/CedricGuillemet/ImGuizmo/issues/310
#if defined(NDEBUG)
        static bool isFirstTime = true;
        if (g_EngineContext->InputSys->GetMouseButtonDown(MouseCode::ButtonLeft) && m_IsWindowHovered &&
            (!ImGuizmo::IsOver() || isFirstTime))
        {
            isFirstTime = false;
#else
        if (g_EngineContext->InputSys->GetMouseButtonDown(MouseCode::ButtonLeft) && m_IsWindowHovered &&
            !ImGuizmo::IsOver())
        {
#endif
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
                if (selectedEntityUUID.has_value() && m_GuizmoOperation != -1)
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
                                         static_cast<ImGuizmo::OPERATION>(m_GuizmoOperation),
                                         m_GuizmoMode,
                                         glm::value_ptr(transform));
                }

                Selector::UnselectAll(SelectionCategory::Viewport);
            }
        }

        if (g_EngineContext->InputSys->GetKeyDown(KeyCode::Q) && m_IsWindowHovered && !ImGuizmo::IsUsing())
        {
            m_GuizmoOperation = -1;
        }

        if (g_EngineContext->InputSys->GetKeyDown(KeyCode::W) && m_IsWindowHovered && !ImGuizmo::IsUsing())
        {
            m_GuizmoOperation = ImGuizmo::OPERATION::TRANSLATE;
        }

        if (g_EngineContext->InputSys->GetKeyDown(KeyCode::E) && m_IsWindowHovered && !ImGuizmo::IsUsing())
        {
            m_GuizmoOperation = ImGuizmo::OPERATION::ROTATE;
        }

        if (g_EngineContext->InputSys->GetKeyDown(KeyCode::R) && m_IsWindowHovered && !ImGuizmo::IsUsing())
        {
            m_GuizmoOperation = ImGuizmo::OPERATION::SCALE;
        }
    }

    void ViewportPanel::DrawToolbar()
    {
        float windowHeight = ImGui::GetWindowHeight();

        float centerHeight = windowHeight * 0.5f;
        float offsetY      = (windowHeight - centerHeight) * 0.5f;
        ImGui::SetCursorPos(ImVec2(0, offsetY));

        ImGui::Indent();
        ImGui::Text("");
        ImGui::SameLine();

        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));

        const ImVec4 selectedColor = ImVec4(0.18f, 0.46f, 0.98f, 1.0f);

        if (m_ViewportMode == ViewportMode::Edit)
        {
            bool selected = false;
            {
                selected = m_GuizmoOperation == -1;
                if (selected)
                    ImGui::PushStyleColor(ImGuiCol_Text, selectedColor);
                ImGui::SameLine();
                if (ImGui::Button(ICON_MDI_HAND_BACK_RIGHT))
                    m_GuizmoOperation = -1;

                if (ImGui::IsItemHovered())
                {
                    ImGui::SetTooltip("Grab the screen to move the viewport.");
                }

                if (selected)
                    ImGui::PopStyleColor();
            }
            ImGui::SameLine();

            {
                selected = m_GuizmoOperation == ImGuizmo::OPERATION::TRANSLATE;
                if (selected)
                    ImGui::PushStyleColor(ImGuiCol_Text, selectedColor);
                ImGui::SameLine();
                if (ImGui::Button(ICON_MDI_ARROW_ALL))
                    m_GuizmoOperation = ImGuizmo::OPERATION::TRANSLATE;

                if (ImGui::IsItemHovered())
                {
                    ImGui::SetTooltip("Translate the selected object.");
                }

                if (selected)
                    ImGui::PopStyleColor();
            }
            ImGui::SameLine();

            {
                selected = m_GuizmoOperation == ImGuizmo::OPERATION::ROTATE;
                if (selected)
                    ImGui::PushStyleColor(ImGuiCol_Text, selectedColor);
                ImGui::SameLine();
                if (ImGui::Button(ICON_MDI_ROTATE_3D_VARIANT))
                    m_GuizmoOperation = ImGuizmo::OPERATION::ROTATE;

                if (ImGui::IsItemHovered())
                {
                    ImGui::SetTooltip("Rotate the selected object.");
                }

                if (selected)
                    ImGui::PopStyleColor();
            }
            ImGui::SameLine();

            {
                selected = m_GuizmoOperation == ImGuizmo::OPERATION::SCALE;
                if (selected)
                    ImGui::PushStyleColor(ImGuiCol_Text, selectedColor);
                ImGui::SameLine();
                if (ImGui::Button(ICON_MDI_ARROW_EXPAND_ALL))
                    m_GuizmoOperation = ImGuizmo::OPERATION::SCALE;

                if (ImGui::IsItemHovered())
                {
                    ImGui::SetTooltip("Scale the selected object.");
                }

                if (selected)
                    ImGui::PopStyleColor();
            }
            ImGui::SameLine();
        }

        bool isSimulating = m_ViewportMode == ViewportMode::Simulating;
        bool isPaused     = m_ViewportMode == ViewportMode::SimulationPaused;
        {
            if (!isSimulating)
            {
                bool playButtonDown = ImGui::Button(ICON_MDI_PLAY);
                if (ImGui::IsItemHovered())
                {
                    ImGui::SetTooltip("Simulate the scene.");
                }

                if (playButtonDown)
                {
                    m_ViewportMode = ViewportMode::Simulating;

                    if (m_SimulatingScene != nullptr)
                    {
                        m_SimulatingScene->SetSimulationStatus(LogicSceneSimulationStatus::Simulating);
                    }
                    else if (m_EditingScene != nullptr)
                    {
                        // Copy scene and set simulating
                        m_SimulatingScene = LogicScene::Copy(m_EditingScene);
                        m_SimulatingScene->SetSimulationStatus(LogicSceneSimulationStatus::Simulating);
                        g_EngineContext->SceneMngr->SetActiveScene(m_SimulatingScene);
                    }
                }
            }

            if (isSimulating)
            {
                bool pauseButtonDown = ImGui::Button(ICON_MDI_PAUSE);
                if (ImGui::IsItemHovered())
                {
                    ImGui::SetTooltip("Pause simulating the scene.");
                }

                if (pauseButtonDown)
                {
                    m_ViewportMode = ViewportMode::SimulationPaused;

                    if (m_SimulatingScene != nullptr)
                    {
                        m_SimulatingScene->SetSimulationStatus(LogicSceneSimulationStatus::Paused);
                    }
                }
            }
            ImGui::SameLine();

            if (isSimulating || isPaused)
            {
                bool stopButtonDown = ImGui::Button(ICON_MDI_STOP);
                if (ImGui::IsItemHovered())
                {
                    ImGui::SetTooltip("Stop simulating the scene.");
                }

                if (stopButtonDown)
                {
                    m_ViewportMode = ViewportMode::Edit;

                    if (m_SimulatingScene != nullptr)
                    {
                        m_SimulatingScene->SetSimulationStatus(LogicSceneSimulationStatus::Stopped);
                        m_SimulatingScene.reset();
                        m_SimulatingScene = nullptr;
                    }

                    if (m_EditingScene != nullptr)
                    {
                        g_EngineContext->SceneMngr->SetActiveScene(m_EditingScene);
                    }
                }
            }
        }
        ImGui::SameLine();

        ImGui::PopStyleColor();
    }
} // namespace SnowLeopardEngine::Editor