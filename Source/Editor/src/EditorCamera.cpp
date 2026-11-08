#include "SnowLeopardEditor/EditorCamera.h"
#include "SnowLeopardEngine/Engine/EngineContext.h"
#include "SnowLeopardEngine/Function/Input/Input.h"

#include <imgui.h>

namespace SnowLeopardEngine::Editor
{
    EditorCamera::EditorCamera()
    {
        m_Transform.Position = {0, 8, 25};
        m_Transform.SetRotationEuler({-15, 0, 0});
        m_Camera.ClearFlags = CameraClearFlags::Skybox;
        m_Camera.Far        = 2000.0f;
    }

    glm::mat4 EditorCamera::GetView() const { return g_EngineContext->CameraSys->GetViewMatrix(m_Transform); }

    glm::mat4 EditorCamera::GetProjection() const { return g_EngineContext->CameraSys->GetProjectionMatrix(m_Camera); }

    void EditorCamera::Basis(glm::vec3& forward, glm::vec3& right, glm::vec3& up) const
    {
        // Rows of the view matrix: the same convention as CameraSystem, whatever the Euler order.
        const auto view = GetView();
        right           = glm::vec3(view[0][0], view[1][0], view[2][0]);
        up              = glm::vec3(view[0][1], view[1][1], view[2][1]);
        forward         = -glm::vec3(view[0][2], view[1][2], view[2][2]);
    }

    void EditorCamera::Focus(const glm::vec3& center, float radius)
    {
        glm::vec3 forward, right, up;
        Basis(forward, right, up);
        const float distance = std::max(radius, 0.5f) / std::tan(glm::radians(m_Camera.FOV) * 0.5f) * 1.2f;
        m_Transform.Position = center - forward * distance;
        m_FocusDistance      = distance;
    }

    void EditorCamera::OnTick(float deltaTime, bool hovered, bool handTool)
    {
        auto&      input = g_EngineContext->InputSys;
        const auto mouse = input->GetMousePosition();
        const auto delta = mouse - m_LastMouse;
        m_LastMouse      = mouse;

        // Start a navigation only over the Viewport; once started it continues outside until the button is up.
        if (m_Mode == Mode::None && hovered)
        {
            const bool alt = input->GetKey(KeyCode::LeftAlt) || input->GetKey(KeyCode::RightAlt);
            if (input->GetMouseButtonDown(MouseCode::ButtonRight))
                m_Mode = Mode::Fly;
            else if (input->GetMouseButtonDown(MouseCode::ButtonMiddle) || (handTool && !alt && input->GetMouseButtonDown(MouseCode::ButtonLeft)))
                m_Mode = Mode::Pan;
            else if (alt && input->GetMouseButtonDown(MouseCode::ButtonLeft))
                m_Mode = Mode::Orbit;
            if (m_Mode != Mode::None)
            {
                // ImGui must not react to the drag (it would move windows or press buttons under the cursor).
                ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_NoMouse;
                if (m_Mode == Mode::Fly)
                    g_EngineContext->WindowSys->SetHideCursor(true);
                return; // the first delta is the jump to where the press happened
            }
        }

        glm::vec3 forward, right, up;
        Basis(forward, right, up);

        switch (m_Mode)
        {
            case Mode::Fly: {
                auto euler = m_Transform.GetRotationEuler();
                euler.y += delta.x * m_LookSpeed;
                euler.x = glm::clamp(euler.x - delta.y * m_LookSpeed, -89.0f, 89.0f);
                m_Transform.SetRotationEuler(euler);
                Basis(forward, right, up);

                glm::vec3 move(0);
                if (input->GetKey(KeyCode::W)) move += forward;
                if (input->GetKey(KeyCode::S)) move -= forward;
                if (input->GetKey(KeyCode::D)) move += right;
                if (input->GetKey(KeyCode::A)) move -= right;
                if (input->GetKey(KeyCode::E)) move += glm::vec3(0, 1, 0);
                if (input->GetKey(KeyCode::Q)) move -= glm::vec3(0, 1, 0);
                const float boost = input->GetKey(KeyCode::LeftShift) ? 3.0f : 1.0f;
                if (glm::dot(move, move) > 0)
                    m_Transform.Position += glm::normalize(move) * m_FlySpeed * boost * deltaTime;
                // The wheel tunes the speed while flying (it would otherwise dolly).
                if (const float wheel = input->GetMouseScrollDelta().y; wheel != 0)
                    SetFlySpeed(m_FlySpeed * std::pow(1.2f, wheel));
                if (!input->GetMouseButton(MouseCode::ButtonRight))
                    m_Mode = Mode::None;
                break;
            }
            case Mode::Pan: {
                // One pixel of drag moves the point under the cursor by one pixel at the focus distance.
                const float pixelSize = 2.0f * m_FocusDistance * std::tan(glm::radians(m_Camera.FOV) * 0.5f) /
                                        std::max(m_Camera.ViewportHeight, 1.0f);
                m_Transform.Position += (-right * delta.x + up * delta.y) * pixelSize;
                if (!input->GetMouseButton(MouseCode::ButtonMiddle) && !input->GetMouseButton(MouseCode::ButtonLeft))
                    m_Mode = Mode::None;
                break;
            }
            case Mode::Orbit: {
                const auto pivot = m_Transform.Position + forward * m_FocusDistance;
                auto       euler = m_Transform.GetRotationEuler();
                euler.y += delta.x * m_LookSpeed * 2;
                euler.x = glm::clamp(euler.x - delta.y * m_LookSpeed * 2, -89.0f, 89.0f);
                m_Transform.SetRotationEuler(euler);
                Basis(forward, right, up);
                m_Transform.Position = pivot - forward * m_FocusDistance;
                if (!input->GetMouseButton(MouseCode::ButtonLeft))
                    m_Mode = Mode::None;
                break;
            }
            case Mode::None:
                if (hovered)
                {
                    // Dolly: a fraction of the distance per notch, so it is useful near and far.
                    if (const float wheel = input->GetMouseScrollDelta().y; wheel != 0)
                    {
                        const float step = std::max(m_FocusDistance * 0.15f, 0.2f) * wheel;
                        m_Transform.Position += forward * step;
                        m_FocusDistance = std::max(m_FocusDistance - step, 0.5f);
                    }
                }
                break;
        }

        if (m_Mode == Mode::None && (ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_NoMouse))
        {
            ImGui::GetIO().ConfigFlags &= ~ImGuiConfigFlags_NoMouse;
            g_EngineContext->WindowSys->SetHideCursor(false);
        }
    }
} // namespace SnowLeopardEngine::Editor
