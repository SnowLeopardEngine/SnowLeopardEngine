#include "SnowLeopardEditor/EditorCamera/EditorCameraScript.h"
#include "SnowLeopardEditor/Selector.h"
#include "SnowLeopardEngine/Engine/EngineContext.h"
#include "SnowLeopardEngine/Function/Input/Input.h"
#include "SnowLeopardEngine/Function/Scene/Components.h"
#include "SnowLeopardEngine/Function/Scene/Entity.h"

#include <imgui.h>

namespace SnowLeopardEngine::Editor
{
    void EditorCameraScript::OnTick(float deltaTime)
    {
        auto& inputSystem = g_EngineContext->InputSys;
        auto& camera      = m_OwnerEntity->GetComponent<CameraComponent>();
        auto& transform   = m_OwnerEntity->GetComponent<TransformComponent>();

        auto mousePosition = inputSystem->GetMousePosition();

        // FreeMove
        if (!m_IsGrabMoveValid)
        {
            if (inputSystem->GetMouseButtonDown(MouseCode::ButtonRight) && m_IsWindowHovered)
            {
                m_LastFrameMousePosition = mousePosition;
                m_IsFreeMoveValid        = true;
            }

            // When pressing mouse button right:
            //  Allow camera rotate around and WASD to move the camera
            if (inputSystem->GetMouseButton(MouseCode::ButtonRight) && m_IsFreeMoveValid)
            {
                // Hide cursor
                g_EngineContext->WindowSys->SetHideCursor(true);

                // Block imgui mouse events
                ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_NoMouse;

                auto offset = mousePosition - m_LastFrameMousePosition;
                offset *= m_Sensitivity;

                auto cameraRotationEuler = transform.GetRotationEuler();

                float yaw   = cameraRotationEuler.y;
                float pitch = cameraRotationEuler.x;

                yaw += offset.x;
                pitch -= offset.y;

                if (pitch > 89.0f)
                {
                    pitch = 89.0f;
                }

                if (pitch < -89.0f)
                {
                    pitch = -89.0f;
                }

                cameraRotationEuler.y = yaw;
                cameraRotationEuler.x = pitch;

                transform.SetRotationEuler(cameraRotationEuler);
                m_LastFrameMousePosition = mousePosition;

                glm::vec3 forward = GetForward(cameraRotationEuler);
                glm::vec3 up      = glm::vec3(0, 1, 0);

                glm::vec3 right = glm::normalize(glm::cross(forward, up));

                float speed = m_BaseSpeed;

                // Speed Up
                if (inputSystem->GetKey(KeyCode::LeftShift))
                {
                    speed *= 2;
                }
                else
                {
                    speed = m_BaseSpeed;
                }

                // Forward
                if (inputSystem->GetKey(KeyCode::W))
                {
                    transform.Position += forward * speed;
                }

                // Backward
                if (inputSystem->GetKey(KeyCode::S))
                {
                    transform.Position -= forward * speed;
                }

                // Left
                if (inputSystem->GetKey(KeyCode::A))
                {
                    transform.Position -= right * speed;
                }

                // Right
                if (inputSystem->GetKey(KeyCode::D))
                {
                    transform.Position += right * speed;
                }

                // Upward
                if (inputSystem->GetKey(KeyCode::Q))
                {
                    transform.Position += up * speed;
                }

                // Downward
                if (inputSystem->GetKey(KeyCode::E))
                {
                    transform.Position -= up * speed;
                }
            }

            if (inputSystem->GetMouseButtonUp(MouseCode::ButtonRight) && m_IsFreeMoveValid)
            {
                // Re-enable ImGui mouse events
                ImGui::GetIO().ConfigFlags &= ~ImGuiConfigFlags_NoMouse;

                g_EngineContext->WindowSys->SetHideCursor(false);
                m_IsFreeMoveValid = false;
            }
        }

        // GrabMove
        if (m_IsGrabMoveEnabled)
        {
            if (inputSystem->GetMouseButtonDown(MouseCode::ButtonLeft) && m_IsWindowHovered)
            {
                m_LastFrameMousePosition = mousePosition;
                m_IsGrabMoveValid        = true;
            }

            // When pressing mouse button left:
            //  drag mouse to grab the view
            if (inputSystem->GetMouseButton(MouseCode::ButtonLeft) && m_IsGrabMoveValid)
            {
                // Block imgui mouse events
                ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_NoMouse;

                auto offset = mousePosition - m_LastFrameMousePosition;
                offset *= m_Sensitivity;

                auto viewMatrix = g_EngineContext->CameraSys->GetViewMatrix(transform);

                auto viewOffset = viewMatrix * glm::vec4(-offset.x, offset.y, 0, 0);

                m_LastFrameMousePosition = mousePosition;
                transform.Position += glm::vec3(viewOffset);
            }

            if (inputSystem->GetMouseButtonUp(MouseCode::ButtonLeft) && m_IsGrabMoveValid)
            {
                // Re-enable ImGui mouse events
                ImGui::GetIO().ConfigFlags &= ~ImGuiConfigFlags_NoMouse;

                m_IsGrabMoveValid = false;
            }
        }

        if (m_IsWindowHovered)
        {
            // Zoom in / out
            auto cameraRotationEuler = transform.GetRotationEuler();

            glm::vec3 forward = GetForward(cameraRotationEuler);

            float yOffset = inputSystem->GetMouseScrollDelta().y;
            transform.Position += yOffset * forward;

            // Focus
            if (inputSystem->GetKeyDown(KeyCode::F))
            {
                auto selectedEntityUUID = Selector::GetLastSelection(SelectionCategory::Viewport);
                if (selectedEntityUUID.has_value())
                {
                    auto selectedEntity =
                        g_EngineContext->SceneMngr->GetActiveScene()->GetEntityWithCoreUUID(selectedEntityUUID.value());
                    if (selectedEntity)
                    {
                        auto selectedEntityTransform = selectedEntity.GetComponent<TransformComponent>();
                        transform.Position           = selectedEntityTransform.Position - 10.0f * forward;
                    }
                }
            }
        }
    }

    // TODO: Move to CameraSystem
    glm::vec3 EditorCameraScript::GetForward(const glm::vec3& cameraRotationEuler) const
    {
        // Calculate forward (Yaw - 90 to adjust)
        glm::vec3 forward;
        forward.x = cos(glm::radians(cameraRotationEuler.y - 90)) * cos(glm::radians(cameraRotationEuler.x));
        forward.y = sin(glm::radians(cameraRotationEuler.x));
        forward.z = sin(glm::radians(cameraRotationEuler.y - 90)) * cos(glm::radians(cameraRotationEuler.x));
        forward   = glm::normalize(forward);

        return forward;
    }
} // namespace SnowLeopardEngine::Editor