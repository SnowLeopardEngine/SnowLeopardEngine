#include "SnowLeopardEngine/Function/GUI/GUISystem.h"
#include "SnowLeopardEngine/Core/Event/UIEvents.h"
#include "SnowLeopardEngine/Engine/EngineContext.h"

namespace SnowLeopardEngine
{
    GUISystem::GUISystem()
    {
        Subscribe(m_LogicSceneLoadedHandler);

        SNOW_LEOPARD_CORE_INFO("[GUISystem] Initialized");

        m_State = SystemState::InitOk;
    };

    GUISystem::~GUISystem()
    {
        SNOW_LEOPARD_CORE_INFO("[GUISystem] Shutting Down...");

        Unsubscribe(m_LogicSceneLoadedHandler);

        m_State = SystemState::ShutdownOk;
    }

    void GUISystem::OnTick(float deltaTime)
    {
        if (m_Scene == nullptr)
        {
            return;
        }

        glm::vec2 mousePos;
        bool      leftMouseButtonDown = false;
        bool      leftMouseButtonUp   = false;

        auto& registry = m_Scene->GetRegistry();

        // Tick Buttons
        if (g_EngineContext->InputSys->GetMouseButtonDown(MouseCode::ButtonLeft))
        {
            leftMouseButtonDown = true;
            mousePos            = g_EngineContext->InputSys->GetMousePosition();
        }

        if (g_EngineContext->InputSys->GetMouseButtonUp(MouseCode::ButtonLeft))
        {
            leftMouseButtonUp = true;
            mousePos          = g_EngineContext->InputSys->GetMousePosition();
        }

        registry.view<UI::RectTransformComponent, UI::ButtonComponent>().each(
            [&registry, mousePos, leftMouseButtonDown, leftMouseButtonUp](
                entt::entity entity, UI::RectTransformComponent& rect, UI::ButtonComponent& button) {
                glm::vec2 tl = glm::vec2(rect.Pos) - glm::vec2(rect.Pivot.x * rect.Size.x, rect.Pivot.y * rect.Size.y);

                if (mousePos.x >= tl.x && mousePos.x < tl.x + rect.Size.x && mousePos.y >= tl.y &&
                    mousePos.y < tl.y + rect.Size.y)
                {
                    if (leftMouseButtonDown)
                    {
                        if (button.TintType == UI::ButtonTintType::Color)
                        {
                            button.TintColor.Current = button.TintColor.Pressed;
                        }
                    }

                    if (leftMouseButtonUp)
                    {
                        // Trigger button clicked event
                        UIButtonClickedEvent buttonClickedEvent(entity);
                        TriggerEvent(buttonClickedEvent);
                    }
                }

                if (leftMouseButtonUp)
                {
                    if (button.TintType == UI::ButtonTintType::Color)
                    {
                        button.TintColor.Current = button.TintColor.Normal;
                    }
                }
            });
    }
} // namespace SnowLeopardEngine