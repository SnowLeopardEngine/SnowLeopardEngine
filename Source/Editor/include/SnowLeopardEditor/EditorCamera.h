#pragma once

#include "SnowLeopardEngine/Core/Math/Math.h"
#include "SnowLeopardEngine/Function/Rendering/Renderer/SceneRenderer.h"
#include "SnowLeopardEngine/Function/Scene/Components.h"

namespace SnowLeopardEngine::Editor
{
    // The Viewport's camera. It is not an entity of the scene: it is never saved into the scene, copied into the
    // simulated scene or seen by scripts; the renderer draws through it with a CameraOverride while editing.
    //
    // Controls (while the Viewport is hovered):
    //   right mouse + move      look around; with W A S D / Q E fly, Shift faster, wheel changes the fly speed
    //   middle mouse + move     pan (also left mouse with the hand tool)
    //   Alt + left mouse        orbit around the focus point
    //   wheel                   dolly towards the focus point
    //   F (Viewport panel)      frame the selection (Focus)
    // Movement is per second, not per frame.
    class EditorCamera
    {
    public:
        EditorCamera();

        // hovered: the Viewport image is under the mouse; handTool: left mouse pans.
        void OnTick(float deltaTime, bool hovered, bool handTool);

        // Looks at `center` from a distance that fits a sphere of `radius`.
        void Focus(const glm::vec3& center, float radius);

        // Navigation in progress (a mouse button that drives the camera is held): picking and tool shortcuts stay off.
        bool IsNavigating() const { return m_Mode != Mode::None; }

        TransformComponent& GetTransform() { return m_Transform; }
        CameraComponent&    GetCamera() { return m_Camera; }
        CameraOverride      GetOverride() { return {&m_Transform, &m_Camera}; }
        glm::mat4           GetView() const;
        glm::mat4           GetProjection() const; // OpenGL convention (ImGuizmo)

        float GetFlySpeed() const { return m_FlySpeed; }
        void  SetFlySpeed(float speed) { m_FlySpeed = glm::clamp(speed, 0.1f, 500.0f); }

    private:
        enum class Mode
        {
            None,
            Fly,
            Pan,
            Orbit
        };

        void Basis(glm::vec3& forward, glm::vec3& right, glm::vec3& up) const;

        TransformComponent m_Transform;
        CameraComponent    m_Camera;
        Mode               m_Mode         = Mode::None;
        glm::vec2          m_LastMouse    = {0, 0};
        float              m_FocusDistance = 20.0f; // orbit / dolly pivot in front of the camera
        float              m_FlySpeed      = 10.0f; // units per second
        float              m_LookSpeed     = 0.15f; // degrees per pixel
    };
} // namespace SnowLeopardEngine::Editor
