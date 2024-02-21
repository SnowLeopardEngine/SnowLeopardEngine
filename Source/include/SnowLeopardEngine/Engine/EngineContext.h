#pragma once

#include "SnowLeopardEngine/Core/Base/SingletonWrapper.h"
#include "SnowLeopardEngine/Core/Event/EventSystem.h"
#include "SnowLeopardEngine/Core/Log/LogSystem.h"
#include "SnowLeopardEngine/Function/Audio/AudioSystem.h"
#include "SnowLeopardEngine/Function/Input/InputSystem.h"
#include "SnowLeopardEngine/Function/Physics/PhysicsSystem.h"
#include "SnowLeopardEngine/Function/Rendering/RenderSystem.h"
#include "SnowLeopardEngine/Function/Scene/SceneManager.h"
#include "SnowLeopardEngine/Function/Window/WindowSystem.h"

namespace SnowLeopardEngine
{
    struct EngineContext
    {
        SingletonWrapper<LogSystem>     LogSys;
        SingletonWrapper<EventSystem>   EventSys;
        SingletonWrapper<AudioSystem>   AudioSys;
        SingletonWrapper<PhysicsSystem> PhysicsSys;
        SingletonWrapper<WindowSystem>  WindowSys;
        SingletonWrapper<InputSystem>   InputSys;
        SingletonWrapper<SceneManager>  SceneMngr;
        SingletonWrapper<RenderSystem>  RenderSys;
    };

    extern EngineContext* g_EngineContext;
} // namespace SnowLeopardEngine
