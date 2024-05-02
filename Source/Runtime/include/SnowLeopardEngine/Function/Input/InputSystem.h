#pragma once

#include "SnowLeopardEngine/Core/Base/Base.h"
#include "SnowLeopardEngine/Core/Base/EngineSubSystem.h"
#include "SnowLeopardEngine/Core/Math/Math.h"
#include "SnowLeopardEngine/Function/Input/Input.h"

namespace SnowLeopardEngine
{
    class InputSystem final : public EngineSubSystem
    {
    public:
        DECLARE_SUBSYSTEM(InputSystem)

        // Keyboard
        bool GetKey(KeyCode key);
        bool GetKeyDown(KeyCode key);
        bool GetKeyUp(KeyCode key);

        // Mouse
        bool GetMouseButton(MouseCode button);
        bool GetMouseButtonDown(MouseCode button);
        bool GetMouseButtonUp(MouseCode button);

        glm::vec2 GetMousePosition() { return m_MousePosition; }
        glm::vec2 GetMousePositionFlipY() { return m_MousePositionFlipY; }
        glm::vec2 GetMouseScrollDelta() { return m_MouseScrollDelta; }

    private:
        template<typename T>
        int ToInt(T code) const
        {
            return static_cast<int>(code);
        }

        // Update states
        void SetKeyState(int key, int action);
        void SetMouseButtonState(int button, int action);
        void SetMousePosition(const glm::vec2& position) { m_MousePosition = position; }
        void SetMousePositionFlipY(const glm::vec2& positionFlipY) { m_MousePositionFlipY = positionFlipY; }
        void SetMouseScrollDelta(const glm::vec2& scrollDelta) { m_MouseScrollDelta = scrollDelta; };

        void ClearStates();

        friend class GLFWWindow;

    private:
        std::map<int, bool> m_KeyDownStates;
        std::map<int, bool> m_KeyUpStates;
        std::map<int, bool> m_CurrentKeyStates;

        std::map<int, bool> m_MouseButtonStates;
        std::map<int, bool> m_MouseButtonDownStates;
        std::map<int, bool> m_MouseButtonUpStates;

        glm::vec2 m_MousePosition      = {0, 0};
        glm::vec2 m_MousePositionFlipY = {0, 0};
        glm::vec2 m_MouseScrollDelta   = {0, 0};
    };
} // namespace SnowLeopardEngine