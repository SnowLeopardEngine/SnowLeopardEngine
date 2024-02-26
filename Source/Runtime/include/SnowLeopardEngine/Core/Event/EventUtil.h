#pragma once

#include "SnowLeopardEngine/Core/Base/Base.h"
#include "SnowLeopardEngine/Engine/EngineContext.h"

namespace SnowLeopardEngine
{
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