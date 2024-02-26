#pragma once

#include "SnowLeopardEngine/Core/Base/EngineSubSystem.h"
#include "SnowLeopardEngine/Core/Event/EventHandler.h"

namespace SnowLeopardEngine
{
    using EventId   = std::string;
    using HandlerId = std::uint64_t;

    class EventSystem final : public EngineSubSystem
    {
    public:
        DECLARE_SUBSYSTEM(EventSystem)

        void Subscribe(EventId eventId, Scope<IEventHandlerWrapper>&& handler, HandlerId handlerId);
        void Unsubscribe(EventId eventId, const std::string& handlerName, HandlerId handlerId);
        void TriggerEvent(const Event& e, HandlerId handlerId);
        void QueueEvent(Scope<Event>&& e, HandlerId handlerId);
        void DispatchEvents();

    private:
        std::vector<std::pair<Scope<Event>, HandlerId>>                       m_EventsQueue;
        std::unordered_map<EventId, std::vector<Scope<IEventHandlerWrapper>>> m_Subscribers;
        std::unordered_map<EventId, std::unordered_map<HandlerId, std::vector<Scope<IEventHandlerWrapper>>>>
            m_SubscribersByHandlerId;
    };
} // namespace SnowLeopardEngine
