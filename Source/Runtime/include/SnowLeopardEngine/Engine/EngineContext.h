#pragma once

#include "SnowLeopardEngine/Core/Base/SingletonWrapper.h"
#include "SnowLeopardEngine/Core/Event/EventSystem.h"
#include "SnowLeopardEngine/Core/Log/LogSystem.h"
#include "SnowLeopardEngine/Function/Audio/AudioSystem.h"
#include "SnowLeopardEngine/Function/Camera/CameraSystem.h"
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
        SingletonWrapper<WindowSystem>  WindowSys;
        SingletonWrapper<InputSystem>   InputSys;
        SingletonWrapper<SceneManager>  SceneMngr;
        SingletonWrapper<PhysicsSystem> PhysicsSys;
        SingletonWrapper<RenderSystem>  RenderSys;
        SingletonWrapper<CameraSystem>  CameraSys;
    };

    extern EngineContext* g_EngineContext;

    template<typename EventType>
    inline void Subscribe(const EventHandler<EventType>& callback, HandlerId handlerId = 0)
    {
        Scope<IEventHandlerWrapper> handler = std::make_unique<EventHandlerWrapper<EventType>>(callback);
        g_EngineContext->EventSys->Subscribe(EventType::GetStaticType(), std::move(handler), handlerId);
    }

    template<typename EventType>
    inline void Unsubscribe(const EventHandler<EventType>& callback, HandlerId handlerId = 0)
    {
        const std::string handlerName = callback.target_type().name();
        g_EngineContext->EventSys->Unsubscribe(EventType::GetStaticType(), handlerName, handlerId);
    }

    inline void TriggerEvent(const Event& triggeredEvent, HandlerId handlerId = 0)
    {
        g_EngineContext->EventSys->TriggerEvent(triggeredEvent, handlerId);
    }

    inline void QueueEvent(Scope<Event>&& queuedEvent, HandlerId handlerId = 0)
    {
        g_EngineContext->EventSys->QueueEvent(std::forward<Scope<Event>>(queuedEvent), handlerId);
    }
} // namespace SnowLeopardEngine

// Core log macros
#define SNOW_LEOPARD_CORE_TRACE(...) ::SnowLeopardEngine::g_EngineContext->LogSys->Trace(true, __VA_ARGS__)
#define SNOW_LEOPARD_CORE_INFO(...) ::SnowLeopardEngine::g_EngineContext->LogSys->Info(true, __VA_ARGS__)
#define SNOW_LEOPARD_CORE_WARN(...) ::SnowLeopardEngine::g_EngineContext->LogSys->Warn(true, __VA_ARGS__)
#define SNOW_LEOPARD_CORE_ERROR(...) ::SnowLeopardEngine::g_EngineContext->LogSys->Error(true, __VA_ARGS__)
#define SNOW_LEOPARD_CORE_CRITICAL(...) ::SnowLeopardEngine::g_EngineContext->LogSys->Critical(true, __VA_ARGS__)

// Client log macros
#define SNOW_LEOPARD_TRACE(...) ::SnowLeopardEngine::g_EngineContext->LogSys->Trace(false, __VA_ARGS__)
#define SNOW_LEOPARD_INFO(...) ::SnowLeopardEngine::g_EngineContext->LogSys->Info(false, __VA_ARGS__)
#define SNOW_LEOPARD_WARN(...) ::SnowLeopardEngine::g_EngineContext->LogSys->Warn(false, __VA_ARGS__)
#define SNOW_LEOPARD_ERROR(...) ::SnowLeopardEngine::g_EngineContext->LogSys->Error(false, __VA_ARGS__)
#define SNOW_LEOPARD_CRITICAL(...) ::SnowLeopardEngine::g_EngineContext->LogSys->Critical(false, __VA_ARGS__)
