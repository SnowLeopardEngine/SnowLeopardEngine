#pragma once

#include "SnowLeopardEngine/Core/Event/EventSystem.h"
#include "SnowLeopardEngine/Core/Log/LogSystem.h"
#include "SnowLeopardEngine/Engine/EngineContext.h"

namespace SnowLeopardEngine
{
    EventSystem::EventSystem()
    {
        SNOW_LEOPARD_CORE_INFO("[EventSystem] Initialized");
        m_State = SystemState::InitOk;
    }

    EventSystem::~EventSystem()
    {
        SNOW_LEOPARD_CORE_INFO("[EventSystem] Shutting Down...");
        m_Subscribers.clear();
        m_State = SystemState::ShutdownOk;
    }

    void EventSystem::Subscribe(EventId eventId, Scope<IEventHandlerWrapper>&& handler, HandlerId handlerId)
    {
        if (handlerId)
        {
            auto subscribers = m_SubscribersByHandlerId.find(eventId);

            if (subscribers != m_SubscribersByHandlerId.end())
            {
                auto& handlersMap = subscribers->second;
                auto  handlers    = handlersMap.find(handlerId);
                if (handlers != handlersMap.end())
                {
                    handlers->second.emplace_back(std::move(handler));
                    return;
                }
            }
            m_SubscribersByHandlerId[eventId][handlerId].emplace_back(std::move(handler));
        }
        else
        {
            auto subscribers = m_Subscribers.find(eventId);
            if (subscribers != m_Subscribers.end())
            {
                auto& handlers = subscribers->second;
                for (auto& it : handlers)
                {
                    if (it->GetTypeName() == handler->GetTypeName())
                    {
                        return;
                    }
                }
                handlers.emplace_back(std::move(handler));
            }
            else
            {
                m_Subscribers[eventId].emplace_back(std::move(handler));
            }
        }
    }

    void EventSystem::Unsubscribe(EventId eventId, const std::string& handlerName, HandlerId handlerId)
    {
        if (handlerId)
        {
            auto subscribers = m_SubscribersByHandlerId.find(eventId);
            if (subscribers != m_SubscribersByHandlerId.end())
            {
                auto& handlersMap = subscribers->second;
                auto  handlers    = handlersMap.find(handlerId);
                if (handlers != handlersMap.end())
                {
                    auto& callbacks = handlers->second;
                    for (auto it = callbacks.begin(); it != callbacks.end(); ++it)
                    {
                        if (it->get()->GetTypeName() == handlerName)
                        {
                            it = callbacks.erase(it);
                            return;
                        }
                    }
                }
            }
        }
        else
        {
            auto handlersIt = m_Subscribers.find(eventId);
            if (handlersIt != m_Subscribers.end())
            {
                auto& handlers = handlersIt->second;
                for (auto it = handlers.begin(); it != handlers.end(); ++it)
                {
                    if (it->get()->GetTypeName() == handlerName)
                    {
                        it = handlers.erase(it);
                        return;
                    }
                }
            }
        }
    }

    void EventSystem::TriggerEvent(const Event& e, HandlerId handlerId)
    {
        for (auto& handler : m_Subscribers[e.GetType()])
        {
            handler->Invoke(e);
        }

        auto& handlersMap = m_SubscribersByHandlerId[e.GetType()];
        auto  handlers    = handlersMap.find(handlerId);
        if (handlers != handlersMap.end())
        {
            for (auto& handler : handlers->second)
            {
                handler->Invoke(e);
            }
        }
    }

    void EventSystem::QueueEvent(Scope<Event>&& e, HandlerId handlerId)
    {
        m_EventsQueue.emplace_back(std::move(e), handlerId);
    }

    void EventSystem::DispatchEvents()
    {
        for (auto eventIt = m_EventsQueue.begin(); eventIt != m_EventsQueue.end();)
        {
            if (!eventIt->first.get()->IsHandled())
            {
                TriggerEvent(*eventIt->first.get(), eventIt->second);
                eventIt = m_EventsQueue.erase(eventIt);
            }
            else
            {
                ++eventIt;
            }
        }
    }
} // namespace SnowLeopardEngine
