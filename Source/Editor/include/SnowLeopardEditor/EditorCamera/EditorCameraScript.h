#pragma once

#include "SnowLeopardEngine/Core/Math/Math.h"
#include "SnowLeopardEngine/Function/NativeScripting/NativeScriptInstance.h"

namespace SnowLeopardEngine::Editor
{
    class EditorCameraScript : public NativeScriptInstance
    {
    public:
        EditorCameraScript() : NativeScriptInstance(true) {}

        virtual void OnTick(float deltaTime) override;

        void SetWindowHovered(bool windowHovered) { m_IsWindowHovered = windowHovered; }
        void SetGrabMoveEnabled(bool grabMoveEnabled) { m_IsGrabMoveEnabled = grabMoveEnabled; }

    private:
        glm::vec3 GetForward(const glm::vec3& cameraRotationEuler) const;

    private:
        float m_Sensitivity = 0.05f;
        float m_BaseSpeed   = 0.1f;

        glm::vec2 m_LastFrameMousePosition;

        bool m_IsWindowHovered   = false;
        bool m_IsFreeMoveValid   = false;
        bool m_IsGrabMoveValid   = false;
        bool m_IsGrabMoveEnabled = false;
    };
} // namespace SnowLeopardEngine::Editor