#include "SnowLeopardEngine/Function/Input/InputSystem.h"
#include "SnowLeopardEngine/Function/Input/Input.h"
#include <map>
#include <GLFW/glfw3.h>


namespace SnowLeopardEngine
{

InputSystem* InputSystem::instance = nullptr; 

InputSystem* InputSystem::GetInstance() {
    if (instance == nullptr) {
        instance = new InputSystem(); 
    }
    return instance; 
}

    //set state
    void InputSystem::SetKeyState(int key, int action) {
        if (action == GLFW_PRESS) {
            if (!currentKeyStates[key]) { 
                KeyDownStates[key] = true;
            }
            currentKeyStates[key] = true;
        } 
        
        else if (action == GLFW_RELEASE) {
            currentKeyStates[key] = false;
            KeyUpStates[key] = true;
        }

    }

    void InputSystem::SetMouseButtonState(int button, int action) {
        if (action == GLFW_PRESS) {
            if (!mouseButtonStates[button]) {
                mouseButtonDownStates[button] = true;
            }
            mouseButtonStates[button] = true;
        } else if (action == GLFW_RELEASE) {
            mouseButtonStates[button] = false;
            mouseButtonUpStates[button] = true;
        }
    }

 //key  state
   bool InputSystem::GetKey(KeyCode key){
        auto it = currentKeyStates.find(ToInt(key));
        return it != currentKeyStates.end() && it->second;
    }

    bool InputSystem:: GetKeyDown(KeyCode key)  {
        auto it = KeyDownStates.find(ToInt(key));
        return it != KeyDownStates.end() && it->second;
    }

    bool InputSystem::GetKeyUp(KeyCode key)  {
        auto it = KeyUpStates.find(ToInt(key));
        return it != KeyUpStates.end() && it->second;
    }


//mouse

   bool InputSystem::GetMouseButton(MouseCode button)  {
        auto it = mouseButtonStates.find(ToInt(button));
        return it != mouseButtonStates.end() && it->second;
    }

    bool InputSystem::GetMouseButtonDown(MouseCode button)  {
        auto it = mouseButtonDownStates.find(ToInt(button));
        return it != mouseButtonDownStates.end() && it->second;
    }

    bool InputSystem::GetMouseButtonUp(MouseCode button)  {
        auto it = mouseButtonUpStates.find(ToInt(button));
        return it != mouseButtonUpStates.end() && it->second;
    }



    void InputSystem::test()
    {
       if(InputSystem::GetMouseButtonDown(MouseCode::ButtonLeft))
        std::cerr<<"1"<<std::endl;
    }




void InputSystem::ClearState() {
        KeyDownStates.clear();
        KeyUpStates.clear();
        mouseButtonDownStates.clear();
        mouseButtonUpStates.clear();
    }
} // namespace SnowLeopardEngine
