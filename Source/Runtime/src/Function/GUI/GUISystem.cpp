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

        auto mousePos = g_EngineContext->InputSys->GetMousePositionFlipY();

        bool leftMouseButton   = false;
        bool leftMouseButtonUp = false;

        auto& registry = m_Scene->GetRegistry();

        // Tick Buttons
        if (g_EngineContext->InputSys->GetMouseButton(MouseCode::ButtonLeft))
        {
            leftMouseButton = true;
        }

        if (g_EngineContext->InputSys->GetMouseButtonUp(MouseCode::ButtonLeft))
        {
            leftMouseButtonUp = true;
        }

        registry.view<UI::RectTransformComponent, UI::ButtonComponent>().each(
            [&registry, mousePos, leftMouseButton, leftMouseButtonUp](
                entt::entity entity, UI::RectTransformComponent& rect, UI::ButtonComponent& button) {
                glm::vec2 bl = glm::vec2(rect.Pos) - glm::vec2(rect.Pivot.x * rect.Size.x, rect.Pivot.y * rect.Size.y);

                if (mousePos.x >= bl.x && mousePos.x < bl.x + rect.Size.x && mousePos.y >= bl.y &&
                    mousePos.y < bl.y + rect.Size.y)
                {
                    if (leftMouseButton)
                    {
                        if (button.TintType == UI::ButtonTintType::Color)
                        {
                            button.TintColor.Current = button.TintColor.Pressed;
                        }
                    }
                    else
                    {
                        if (button.TintType == UI::ButtonTintType::Color)
                        {
                            button.TintColor.Current = button.TintColor.Hovered;
                        }
                    }

                    if (leftMouseButtonUp)
                    {
                        // Trigger button clicked event
                        UIButtonClickedEvent buttonClickedEvent(entity);
                        TriggerEvent(buttonClickedEvent);
                    }
                }
                else
                {
                    if (button.TintType == UI::ButtonTintType::Color)
                    {
                        button.TintColor.Current = button.TintColor.Normal;
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