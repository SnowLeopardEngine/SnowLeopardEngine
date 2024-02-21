#pragma once

#include "SnowLeopardEngine/Core/Base/Base.h"
#include "SnowLeopardEngine/Core/Base/EngineSubSystem.h"
#include "SnowLeopardEngine/Function/Input/Input.h"

namespace SnowLeopardEngine
{
    class InputSystem final : public EngineSubSystem
    {
    public:
        DECLARE_SUBSYSTEM(InputSystem)

        // Update states
        void SetKeyState(int key, int action);
        void SetMouseButtonState(int button, int action);

        // Keyboard
        bool GetKey(KeyCode key);
        bool GetKeyDown(KeyCode key);
        bool GetKeyUp(KeyCode key);

        // Mouse
        bool GetMouseButton(MouseCode button);
        bool GetMouseButtonDown(MouseCode button);
        bool GetMouseButtonUp(MouseCode button);

        void ClearStates();

    private:
        template<typename T>
        int ToInt(T code) const
        {
            return static_cast<int>(code);
        }

    private:
        std::map<int, bool> m_KeyDownStates;
        std::map<int, bool> m_KeyUpStates;
        std::map<int, bool> m_CurrentKeyStates;

        std::map<int, bool> m_MouseButtonStates;
        std::map<int, bool> m_MouseButtonDownStates;
        std::map<int, bool> m_MouseButtonUpStates;
    };
} // namespace SnowLeopardEngine