#include "SnowLeopardEditor/EditorCamera/EditorCameraScript.h"
#include "SnowLeopardEngine/Engine/EngineContext.h"
#include "SnowLeopardEngine/Function/Input/Input.h"
#include "SnowLeopardEngine/Function/Scene/Components.h"
#include "SnowLeopardEngine/Function/Scene/Entity.h"

#include <imgui.h>

namespace SnowLeopardEngine::Editor
{
    void EditorCameraScript::OnTick(float deltaTime)
    {
        auto& inputSystem   = g_EngineContext->InputSys;
        auto  mousePosition = inputSystem->GetMousePosition();

        if (m_Mode == EditorCameraMode::FreeMove)
        {
            if (inputSystem->GetMouseButtonDown(MouseCode::ButtonRight) && m_IsWindowHovered)
            {
                m_LastFrameMousePosition = mousePosition;
                m_IsFreeMoveValid                = true;
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

                if (m_IsFirstTime)
                {
                    offset        = {0, 0};
                    m_IsFirstTime = false;
                }

                auto& transform           = m_OwnerEntity->GetComponent<TransformComponent>();
                auto  cameraRotationEuler = transform.GetRotationEuler();

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

                // Calculate forward (Yaw - 90 to adjust)
                glm::vec3 forward;
                forward.x = cos(glm::radians(cameraRotationEuler.y - 90)) * cos(glm::radians(cameraRotationEuler.x));
                forward.y = sin(glm::radians(cameraRotationEuler.x));
                forward.z = sin(glm::radians(cameraRotationEuler.y - 90)) * cos(glm::radians(cameraRotationEuler.x));
                forward   = glm::normalize(forward);
                glm::vec3 up = glm::vec3(0, 1, 0);

                glm::vec3 right = glm::normalize(glm::cross(forward, up));

                if (inputSystem->GetKey(KeyCode::W))
                {
                    transform.Position += forward * m_Speed;
                }

                if (inputSystem->GetKey(KeyCode::S))
                {
                    transform.Position -= forward * m_Speed;
                }

                if (inputSystem->GetKey(KeyCode::A))
                {
                    transform.Position -= right * m_Speed;
                }

                if (inputSystem->GetKey(KeyCode::D))
                {
                    transform.Position += right * m_Speed;
                }

                if (inputSystem->GetKey(KeyCode::Q))
                {
                    transform.Position += up * m_Speed;
                }

                if (inputSystem->GetKey(KeyCode::E))
                {
                    transform.Position -= up * m_Speed;
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
        else if (m_Mode == EditorCameraMode::GrabMove)
        {
            // TODO: GrabMove
        }
    }
} // namespace SnowLeopardEngine::Editor