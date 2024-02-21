#pragma once
#include <map>
#include "SnowLeopardEngine/Function/Input/Input.h"
namespace SnowLeopardEngine{

class InputSystem {
private:
    
std::map<int, bool> KeyDownStates;
std::map<int, bool> KeyUpStates;
std::map<int, bool> currentKeyStates;

 std::map<int, bool> mouseButtonStates;
 std::map<int, bool> mouseButtonDownStates;
 std::map<int, bool> mouseButtonUpStates;


 template<typename T>
    int ToInt(T code) const {
        return static_cast<int>(code);
    }
 


static InputSystem* instance; 

InputSystem() {} 



public:
  static InputSystem* GetInstance();

  //state update
  void SetKeyState(int key,int action);
  void SetMouseButtonState(int button,int action);
  //input
  bool GetKey(KeyCode key);
  bool GetKeyDown(KeyCode key);
  bool GetKeyUp(KeyCode key);
  

  //Mouse
  bool GetMouseButton(MouseCode button);
  bool GetMouseButtonDown(MouseCode button);
  bool GetMouseButtonUp(MouseCode button);
  
  void ClearState();

  void test();
};
}