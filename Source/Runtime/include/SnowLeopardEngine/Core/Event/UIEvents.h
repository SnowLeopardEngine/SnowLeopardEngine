#pragma once

#include "SnowLeopardEngine/Core/Event/Event.h"

#include <entt/fwd.hpp>

namespace SnowLeopardEngine
{
    class UIButtonClickedEvent final : public Event
    {
    public:
        DECLARE_EVENT_TYPE(UIButtonClickedEvent)

        explicit UIButtonClickedEvent(entt::entity buttonEntity) : m_ButtonEntity(buttonEntity) {}

        entt::entity GetButtonEntity() const { return m_ButtonEntity; }

        virtual const std::string ToString() const override { return "UIButtonClickedEvent"; }

    private:
        entt::entity m_ButtonEntity;
    };
} // namespace SnowLeopardEngine