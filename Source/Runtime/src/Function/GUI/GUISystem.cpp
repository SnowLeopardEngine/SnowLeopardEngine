#include "SnowLeopardEngine/Function/GUI/GUISystem.h"
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
        }

        registry.view<UI::RectTransformComponent, UI::ButtonComponent>().each(
            [mousePos, leftMouseButtonDown](
                entt::entity entity, UI::RectTransformComponent& rect, UI::ButtonComponent& button) {
                if (leftMouseButtonDown)
                {
                    glm::vec2 tl = rect.Pos - glm::dot(rect.Pivot, rect.Size);

                    if (mousePos.x >= tl.x && mousePos.x < tl.x + rect.Size.x && mousePos.y >= tl.y &&
                        mousePos.y < tl.y + rect.Size.y)
                    {
                        SNOW_LEOPARD_CORE_INFO("[GUISystem] Button down!");

                        if (button.TintType == UI::ButtonTintType::Color)
                        {
                            button.TintColor.Current = button.TintColor.Pressed;
                        }
                    }
                }

                // TODO: Render button

                // TODO: Switch color to normal
            });
    }
} // namespace SnowLeopardEngine