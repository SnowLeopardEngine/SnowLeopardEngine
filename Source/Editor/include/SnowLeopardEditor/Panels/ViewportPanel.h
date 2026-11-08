#pragma once

#include "SnowLeopardEditor/PanelBase.h"
#include "SnowLeopardEngine/Core/Base/Base.h"
#include "SnowLeopardEngine/Core/Math/Math.h"

#include <ImGuizmo.h>

namespace SnowLeopardEngine::Editor
{
    // The scene view: renders the active scene into a texture of the panel's size (through the editor camera while
    // editing), picks entities with the renderer's entity-id buffer, and moves the selection with ImGuizmo.
    class ViewportPanel final : public PanelBase
    {
    public:
        virtual void Init(const PanelCommonInitInfo& initInfo) override final;
        virtual void OnFixedTick() override final;
        virtual void OnTick(float deltaTime) override final;

    private:
        void DrawToolbar();
        void DrawStats(const glm::vec2& min, const glm::vec2& size);
        void HandleShortcuts();
        void HandlePicking(const glm::vec2& min, const glm::vec2& size);
        void DrawGizmo(const glm::vec2& min, const glm::vec2& size);

    private:
        // -1: no gizmo (the hand tool: left mouse pans the camera)
        int            m_GizmoOperation = ImGuizmo::OPERATION::TRANSLATE;
        ImGuizmo::MODE m_GizmoMode      = ImGuizmo::MODE::LOCAL;
        bool           m_ShowStats      = true;

        bool      m_IsHovered        = false;
        bool      m_PickPending      = false;
        bool      m_PickAdditive     = false;
        glm::vec2 m_PressPosition    = {0, 0};
        bool      m_PressedInside    = false;
        bool      m_GizmoWasUsing    = false;
    };
} // namespace SnowLeopardEngine::Editor
