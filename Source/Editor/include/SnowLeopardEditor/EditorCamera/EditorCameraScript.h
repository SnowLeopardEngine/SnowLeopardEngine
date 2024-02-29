#pragma once

#include "SnowLeopardEngine/Core/Math/Math.h"
#include "SnowLeopardEngine/Function/NativeScripting/NativeScriptInstance.h"

namespace SnowLeopardEngine::Editor
{
    enum class EditorCameraMode : uint8_t
    {
        FreeMove = 0,
        GrabMove
    };

    class EditorCameraScript : public NativeScriptInstance
    {
    public:
        virtual void OnTick(float deltaTime) override;

        void SetWindowHovered(bool windowHovered) { m_IsWindowHovered = windowHovered; }

    private:
        glm::vec3 GetForward(const glm::vec3& cameraRotationEuler) const;

    private:
        float m_Sensitivity = 0.05f;
        float m_Speed       = 0.1f;

        bool      m_IsFirstTime = true;
        glm::vec2 m_LastFrameMousePosition;

        bool m_IsWindowHovered = false;
        bool m_IsFreeMoveValid = false;

        EditorCameraMode m_Mode = EditorCameraMode::FreeMove;
    };
} // namespace SnowLeopardEngine::Editor