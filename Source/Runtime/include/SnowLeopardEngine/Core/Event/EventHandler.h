#pragma once

#include "SnowLeopardEngine/Core/Event/Event.h"

namespace SnowLeopardEngine
{
    template<typename EventType>
    using EventHandler = std::function<void(const EventType& e)>;

    class IEventHandlerWrapper
    {
    public:
        virtual ~IEventHandlerWrapper() = default;

        void Invoke(const Event& e) { InvokeInternal(e); }

        virtual std::string GetTypeName() const = 0;

    private:
        virtual void InvokeInternal(const Event& e) = 0;
    };

    template<typename EventType>
    class EventHandlerWrapper : public IEventHandlerWrapper
    {
    public:
        explicit EventHandlerWrapper(const EventHandler<EventType>& handler) :
            m_Handler(handler), m_HandlerType(m_Handler.target_type().name()) {};

    private:
        void InvokeInternal(const Event& e) override
        {
            if (e.GetType() == EventType::GetStaticType())
            {
                m_Handler(static_cast<const EventType&>(e));
            }
        }

        std::string GetTypeName() const override { return m_HandlerType; }

        EventHandler<EventType> m_Handler;
        const std::string       m_HandlerType;
    };
} // namespace SnowLeopardEngine
