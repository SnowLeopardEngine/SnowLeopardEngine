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

// Core log macros
#define SNOW_LEOPARD_CORE_TRACE(...) ::SnowLeopardEngine::g_EngineContext->LogSys->GetCoreLogger()->trace(__VA_ARGS__)
#define SNOW_LEOPARD_CORE_INFO(...) ::SnowLeopardEngine::g_EngineContext->LogSys->GetCoreLogger()->info(__VA_ARGS__)
#define SNOW_LEOPARD_CORE_WARN(...) ::SnowLeopardEngine::g_EngineContext->LogSys->GetCoreLogger()->warn(__VA_ARGS__)
#define SNOW_LEOPARD_CORE_ERROR(...) ::SnowLeopardEngine::g_EngineContext->LogSys->GetCoreLogger()->error(__VA_ARGS__)
#define SNOW_LEOPARD_CORE_CRITICAL(...) \
    ::SnowLeopardEngine::g_EngineContext->LogSys->GetCoreLogger()->critical(__VA_ARGS__)

// Client log macros
#define SNOW_LEOPARD_TRACE(...) ::SnowLeopardEngine::g_EngineContext->LogSys->GetClientLogger()->trace(__VA_ARGS__)
#define SNOW_LEOPARD_INFO(...) ::SnowLeopardEngine::g_EngineContext->LogSys->GetClientLogger()->info(__VA_ARGS__)
#define SNOW_LEOPARD_WARN(...) ::SnowLeopardEngine::g_EngineContext->LogSys->GetClientLogger()->warn(__VA_ARGS__)
#define SNOW_LEOPARD_ERROR(...) ::SnowLeopardEngine::g_EngineContext->LogSys->GetClientLogger()->error(__VA_ARGS__)
#define SNOW_LEOPARD_CRITICAL(...) \
    ::SnowLeopardEngine::g_EngineContext->LogSys->GetClientLogger()->critical(__VA_ARGS__)
