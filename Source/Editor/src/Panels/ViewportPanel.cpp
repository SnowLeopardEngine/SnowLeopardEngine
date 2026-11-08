#include "SnowLeopardEditor/Panels/ViewportPanel.h"
#include "SnowLeopardEditor/EditorContext.h"
#include "SnowLeopardEngine/Engine/EngineContext.h"
#include "SnowLeopardEngine/Function/Input/Input.h"
#include "SnowLeopardEngine/Function/Scene/Components.h"

#include "IconsMaterialDesignIcons.h"
#include <imgui.h>

namespace SnowLeopardEngine::Editor
{
    void ViewportPanel::Init(const PanelCommonInitInfo& initInfo)
    {
        // The scene is rendered into a texture that this panel draws, with the editor overlay (entity ids for
        // picking, the selection outline).
        auto& renderSystem = g_EngineContext->RenderSys;
        renderSystem->SetRenderToTexture(true);
        auto settings          = renderSystem->GetSceneRenderer()->GetSettings();
        settings.EditorOverlay = true;
        renderSystem->GetSceneRenderer()->SetSettings(settings);
    }

    void ViewportPanel::OnFixedTick()
    {
        g_EngineContext->SceneMngr->OnFixedTick();
        if (EditorContext::GetMode() == EditorMode::Playing)
        {
            g_EngineContext->PhysicsSys->OnFixedTick();
        }
    }

    void ViewportPanel::OnTick(float deltaTime)
    {
        g_EngineContext->SceneMngr->OnTick(deltaTime);

        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
        ImGui::Begin("Viewport");
        ImGui::PopStyleVar();

        ImGui::BeginChild("ViewportToolbar", ImVec2(0, 32), false, ImGuiWindowFlags_NoScrollbar);
        DrawToolbar();
        ImGui::EndChild();

        ImGui::BeginChild("ViewportMain", ImVec2(0, 0), false, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoMove);

        // Nothing has been drawn into the child yet: the cursor is the content region's top-left corner.
        const auto      cursor = ImGui::GetCursorScreenPos();
        const auto      avail  = ImGui::GetContentRegionAvail();
        const glm::vec2 min {cursor.x, cursor.y};
        const glm::vec2 size {std::floor(avail.x), std::floor(avail.y)};

        auto& renderSystem = g_EngineContext->RenderSys;
        if (!g_EngineContext->WindowSys->IsMinimized() && size.x >= 1.0f && size.y >= 1.0f)
        {
            renderSystem->UpdateViewport({.Offset = {.X = 0, .Y = 0},
                                          .Extent = {.Width = uint32_t(size.x), .Height = uint32_t(size.y)}});
            renderSystem->OnTick(deltaTime);
            if (const auto texture = renderSystem->GetOutputTextureID(); texture != 0)
            {
                // The viewport texture's rows run top to bottom, like ImGui's uv.
                ImGui::Image(static_cast<ImTextureID>(texture), avail);
                ImGui::SetCursorScreenPos(cursor);
            }
        }
        m_IsHovered = ImGui::IsWindowHovered();

        // The editor camera only moves while editing; playing shows the scene's own camera.
        auto& camera = EditorContext::GetCamera();
        // The gizmo needs the aspect before the renderer sets it (first frame, resizes).
        camera.GetCamera().ViewportWidth  = std::max(size.x, 1.0f);
        camera.GetCamera().ViewportHeight = std::max(size.y, 1.0f);
        if (EditorContext::GetMode() == EditorMode::Edit)
        {
            camera.OnTick(deltaTime, m_IsHovered, m_GizmoOperation == -1);
        }

        // Outline what is selected (the renderer resolves entities of the scene it draws).
        std::vector<entt::entity> selected;
        for (auto entity : EditorContext::GetSelection())
        {
            selected.push_back(entity);
        }
        renderSystem->GetSceneRenderer()->SetSelection(selected);

        if (EditorContext::GetMode() != EditorMode::Playing)
        {
            HandleShortcuts();
            DrawGizmo(min, size);
            HandlePicking(min, size);
        }
        if (m_ShowStats)
        {
            DrawStats(min, size);
        }

        ImGui::EndChild();
        ImGui::End();
    }

    void ViewportPanel::HandleShortcuts()
    {
        // Single keys only over the Viewport, never while flying (W A S D Q E move the camera) or typing.
        if (!m_IsHovered || EditorContext::GetCamera().IsNavigating() || ImGui::GetIO().WantTextInput || ImGuizmo::IsUsing())
        {
            return;
        }
        if (ImGui::IsKeyPressed(ImGuiKey_Q, false))
            m_GizmoOperation = -1;
        if (ImGui::IsKeyPressed(ImGuiKey_W, false))
            m_GizmoOperation = ImGuizmo::OPERATION::TRANSLATE;
        if (ImGui::IsKeyPressed(ImGuiKey_E, false))
            m_GizmoOperation = ImGuizmo::OPERATION::ROTATE;
        if (ImGui::IsKeyPressed(ImGuiKey_R, false))
            m_GizmoOperation = ImGuizmo::OPERATION::SCALE;
        if (ImGui::IsKeyPressed(ImGuiKey_T, false))
            m_GizmoMode = m_GizmoMode == ImGuizmo::LOCAL ? ImGuizmo::WORLD : ImGuizmo::LOCAL;
        if (ImGui::IsKeyPressed(ImGuiKey_F, false))
            EditorContext::FocusSelection();
    }

    void ViewportPanel::HandlePicking(const glm::vec2& min, const glm::vec2& size)
    {
        auto* renderer = g_EngineContext->RenderSys->GetSceneRenderer();

        // The result of last frame's request (the frame is read back after it is submitted).
        if (m_PickPending)
        {
            if (auto picked = renderer->TakePickResult())
            {
                m_PickPending = false;
                auto scene    = EditorContext::GetActiveScene();
                if (*picked != entt::null && scene != nullptr && scene->GetRegistry().valid(*picked))
                {
                    Entity entity(*picked, scene.get());
                    if (m_PickAdditive && EditorContext::IsSelected(entity))
                        EditorContext::Deselect(entity);
                    else
                        EditorContext::Select(entity, m_PickAdditive);
                }
                else if (!m_PickAdditive)
                {
                    EditorContext::ClearSelection();
                }
            }
        }

        // A click: press and release at (almost) the same place, not on the gizmo, not while navigating. A drag that
        // started on the gizmo or moved the camera selects nothing.
        const auto mouse = ImGui::GetMousePos();
        const glm::vec2 local {mouse.x - min.x, mouse.y - min.y};
        if (ImGui::IsMouseClicked(ImGuiMouseButton_Left))
        {
            m_PressPosition = local;
            m_PressedInside = m_IsHovered && !ImGuizmo::IsOver() && !ImGui::GetIO().KeyAlt && m_GizmoOperation != -1;
        }
        if (ImGui::IsMouseReleased(ImGuiMouseButton_Left) && m_PressedInside)
        {
            m_PressedInside = false;
            const bool inside = local.x >= 0 && local.y >= 0 && local.x < size.x && local.y < size.y;
            if (inside && glm::distance(local, m_PressPosition) < 4.0f && !m_GizmoWasUsing)
            {
                renderer->RequestPick(glm::uvec2(local));
                m_PickPending  = true;
                m_PickAdditive = ImGui::GetIO().KeyCtrl || ImGui::GetIO().KeyShift;
            }
        }
        m_GizmoWasUsing = ImGuizmo::IsUsing() || (m_GizmoWasUsing && ImGui::IsMouseDown(ImGuiMouseButton_Left));
    }

    void ViewportPanel::DrawGizmo(const glm::vec2& min, const glm::vec2& size)
    {
        auto   scene    = EditorContext::GetActiveScene();
        Entity selected = EditorContext::GetPrimarySelection();
        if (scene == nullptr || !selected || m_GizmoOperation == -1)
        {
            return;
        }

        auto& camera = EditorContext::GetCamera();
        ImGuizmo::SetOrthographic(false);
        ImGuizmo::SetDrawlist();
        ImGuizmo::SetRect(min.x, min.y, size.x, size.y);
        ImGuizmo::Enable(!camera.IsNavigating());

        const auto view       = camera.GetView();
        const auto projection = camera.GetProjection();
        // The gizmo works on the world transform; the result goes back as the local transform under the parent.
        glm::mat4  world      = scene->GetWorldTransform(selected);

        const bool  snap       = ImGui::GetIO().KeyCtrl;
        const float snapValue  = m_GizmoOperation == ImGuizmo::OPERATION::ROTATE ? 15.0f : 0.5f;
        const float snaps[3]   = {snapValue, snapValue, snapValue};
        // Scaling only makes sense in local space.
        const auto mode = m_GizmoOperation == ImGuizmo::OPERATION::SCALE ? ImGuizmo::LOCAL : m_GizmoMode;

        if (ImGuizmo::Manipulate(glm::value_ptr(view),
                                 glm::value_ptr(projection),
                                 static_cast<ImGuizmo::OPERATION>(m_GizmoOperation),
                                 mode,
                                 glm::value_ptr(world),
                                 nullptr,
                                 snap ? snaps : nullptr))
        {
            const glm::mat4 local = glm::inverse(scene->GetParentWorldTransform(selected)) * world;
            auto&           transform = selected.GetComponent<TransformComponent>();
            glm::vec3       rotation;
            ImGuizmo::DecomposeMatrixToComponents(glm::value_ptr(local),
                                                  glm::value_ptr(transform.Position),
                                                  glm::value_ptr(rotation),
                                                  glm::value_ptr(transform.Scale));
            transform.SetRotationEuler(rotation);
        }
    }

    void ViewportPanel::DrawStats(const glm::vec2& min, const glm::vec2& size)
    {
        auto&       renderSystem = g_EngineContext->RenderSys;
        const auto& stats        = renderSystem->GetSceneRenderer()->GetStatistics();
        const auto& io           = ImGui::GetIO();

        char text[256];
        std::snprintf(text,
                      sizeof(text),
                      "%.0f FPS  %.2f ms\nGPU %.2f ms\n%u x %u\n%u objects  %u materials",
                      io.Framerate,
                      1000.0f / std::max(io.Framerate, 1.0f),
                      renderSystem->GetFrameGpuMs(),
                      uint32_t(size.x),
                      uint32_t(size.y),
                      stats.Instances,
                      stats.Materials);
        auto*       drawList = ImGui::GetWindowDrawList();
        const auto  extent   = ImGui::CalcTextSize(text);
        const ImVec2 origin(min.x + size.x - extent.x - 14, min.y + 8);
        drawList->AddRectFilled(ImVec2(origin.x - 6, origin.y - 4), ImVec2(origin.x + extent.x + 6, origin.y + extent.y + 4), IM_COL32(0, 0, 0, 140), 4);
        drawList->AddText(origin, IM_COL32(230, 230, 230, 255), text);
    }

    void ViewportPanel::DrawToolbar()
    {
        ImGui::SetCursorPos(ImVec2(8, 4));
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
        const ImVec4 selectedColor = ImVec4(0.18f, 0.46f, 0.98f, 1.0f);

        auto toolButton = [&](const char* icon, int operation, const char* tooltip) {
            const bool selected = m_GizmoOperation == operation;
            if (selected)
                ImGui::PushStyleColor(ImGuiCol_Text, selectedColor);
            if (ImGui::Button(icon))
                m_GizmoOperation = operation;
            if (selected)
                ImGui::PopStyleColor();
            if (ImGui::IsItemHovered())
                ImGui::SetTooltip("%s", tooltip);
            ImGui::SameLine();
        };

        const auto mode = EditorContext::GetMode();
        ImGui::BeginDisabled(mode == EditorMode::Playing);
        toolButton(ICON_MDI_HAND_BACK_RIGHT, -1, "Pan the view (Q)");
        toolButton(ICON_MDI_ARROW_ALL, ImGuizmo::OPERATION::TRANSLATE, "Move the selection (W)");
        toolButton(ICON_MDI_ROTATE_3D_VARIANT, ImGuizmo::OPERATION::ROTATE, "Rotate the selection (E)");
        toolButton(ICON_MDI_ARROW_EXPAND_ALL, ImGuizmo::OPERATION::SCALE, "Scale the selection (R)");
        if (ImGui::Button(m_GizmoMode == ImGuizmo::LOCAL ? ICON_MDI_CUBE_OUTLINE " Local" : ICON_MDI_EARTH " World"))
            m_GizmoMode = m_GizmoMode == ImGuizmo::LOCAL ? ImGuizmo::WORLD : ImGuizmo::LOCAL;
        if (ImGui::IsItemHovered())
            ImGui::SetTooltip("Gizmo space (T). Hold Ctrl to snap.");
        ImGui::EndDisabled();

        // Play controls in the middle.
        const float buttons = 3 * 30.0f;
        ImGui::SameLine(std::max(ImGui::GetCursorPosX() + 16, (ImGui::GetWindowWidth() - buttons) * 0.5f));
        if (mode != EditorMode::Playing)
        {
            if (ImGui::Button(ICON_MDI_PLAY))
                EditorContext::Play();
            if (ImGui::IsItemHovered())
                ImGui::SetTooltip(mode == EditorMode::Paused ? "Resume (Ctrl+P)" : "Play a copy of the scene (Ctrl+P)");
        }
        else
        {
            if (ImGui::Button(ICON_MDI_PAUSE))
                EditorContext::Pause();
            if (ImGui::IsItemHovered())
                ImGui::SetTooltip("Pause");
        }
        ImGui::SameLine();
        ImGui::BeginDisabled(mode == EditorMode::Edit);
        if (ImGui::Button(ICON_MDI_STOP))
            EditorContext::Stop();
        if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled))
            ImGui::SetTooltip("Stop and return to the edited scene (Ctrl+P)");
        ImGui::EndDisabled();

        // Right side: camera speed and the stats overlay.
        ImGui::SameLine(std::max(ImGui::GetCursorPosX() + 16, ImGui::GetWindowWidth() - 190));
        auto& camera = EditorContext::GetCamera();
        float speed  = camera.GetFlySpeed();
        ImGui::SetNextItemWidth(90);
        if (ImGui::DragFloat("##speed", &speed, 0.2f, 0.1f, 500.0f, ICON_MDI_SPEEDOMETER " %.1f"))
            camera.SetFlySpeed(speed);
        if (ImGui::IsItemHovered())
            ImGui::SetTooltip("Fly speed (units / s); the wheel changes it while flying");
        ImGui::SameLine();
        if (m_ShowStats)
            ImGui::PushStyleColor(ImGuiCol_Text, selectedColor);
        if (ImGui::Button(ICON_MDI_CHART_BAR))
            m_ShowStats = !m_ShowStats;
        if (m_ShowStats)
            ImGui::PopStyleColor();
        if (ImGui::IsItemHovered())
            ImGui::SetTooltip("Statistics overlay");

        ImGui::PopStyleColor();
    }
} // namespace SnowLeopardEngine::Editor
