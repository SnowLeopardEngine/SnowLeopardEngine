#include "SnowLeopardEngine/Function/Input/InputSystem.h"
#include "SnowLeopardEngine/Engine/EngineContext.h"
#include "SnowLeopardEngine/Function/Input/Input.h"

namespace SnowLeopardEngine
{
    // set state
    InputSystem::InputSystem()
    {
        SNOW_LEOPARD_CORE_INFO("[InputSystem] Initialized");
        m_State = SystemState::InitOk;
    }

    InputSystem::~InputSystem()
    {
        SNOW_LEOPARD_CORE_INFO("[InputSystem] Shutting Down...");
        m_State = SystemState::ShutdownOk;
    }

    void InputSystem::SetKeyState(int key, int action)
    {
        if (action == ToInt(InputAction::Press))
        {
            if (!m_CurrentKeyStates[key])
            {
                m_KeyDownStates[key] = true;
            }
            m_CurrentKeyStates[key] = true;
        }

        else if (action == ToInt(InputAction::Release))
        {
            m_CurrentKeyStates[key] = false;
            m_KeyUpStates[key]      = true;
        }
    }

    void InputSystem::SetMouseButtonState(int button, int action)
    {
        if (action == ToInt(InputAction::Press))
        {
            if (!m_MouseButtonStates[button])
            {
                m_MouseButtonDownStates[button] = true;
            }
            m_MouseButtonStates[button] = true;
        }
        else if (action == ToInt(InputAction::Release))
        {
            m_MouseButtonStates[button]   = false;
            m_MouseButtonUpStates[button] = true;
        }
    }

    // key  state

    bool InputSystem::GetKey(KeyCode key)
    {
        auto it = m_CurrentKeyStates.find(ToInt(key));
        return it != m_CurrentKeyStates.end() && it->second;
    }

    bool InputSystem::GetKeyDown(KeyCode key)
    {
        auto it = m_KeyDownStates.find(ToInt(key));
        return it != m_KeyDownStates.end() && it->second;
    }

    bool InputSystem::GetKeyUp(KeyCode key)
    {
        auto it = m_KeyUpStates.find(ToInt(key));
        return it != m_KeyUpStates.end() && it->second;
    }

    // mouse

    bool InputSystem::GetMouseButton(MouseCode button)
    {
        auto it = m_MouseButtonStates.find(ToInt(button));
        return it != m_MouseButtonStates.end() && it->second;
    }

    bool InputSystem::GetMouseButtonDown(MouseCode button)
    {
        auto it = m_MouseButtonDownStates.find(ToInt(button));
        return it != m_MouseButtonDownStates.end() && it->second;
    }

    bool InputSystem::GetMouseButtonUp(MouseCode button)
    {
        auto it = m_MouseButtonUpStates.find(ToInt(button));
        return it != m_MouseButtonUpStates.end() && it->second;
    }

    void InputSystem::ClearStates()
    {
        m_KeyDownStates.clear();
        m_KeyUpStates.clear();
        m_MouseButtonDownStates.clear();
        m_MouseButtonUpStates.clear();
    }
} // namespace SnowLeopardEngine
