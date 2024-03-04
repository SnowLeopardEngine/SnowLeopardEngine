#pragma once

#include "SnowLeopardEditor/EditorCamera/EditorCameraScript.h"
#include "SnowLeopardEditor/PanelBase.h"
#include "SnowLeopardEngine/Core/Base/Base.h"
#include "SnowLeopardEngine/Function/Rendering/RHI/FrameBuffer.h"
#include <SnowLeopardEngine/Function/Scene/Entity.h>

#include <ImGuizmo.h>

namespace SnowLeopardEngine::Editor
{
    class ViewportPanel final : public PanelBase
    {
    public:
        virtual void Init() override final;
        virtual void OnFixedTick() override final;
        virtual void OnTick(float deltaTime) override final;

    private:
        void HandleInput();
        void DrawToolbar();

    private:
        Ref<FrameBuffer> m_RenderTarget = nullptr;
        glm::vec2        m_ViewportSize = {1024, 1024};

        Entity                  m_EditorCamera       = {};
        Ref<EditorCameraScript> m_EditorCameraScript = nullptr;

        int            m_GuizmoOperation = ImGuizmo::OPERATION::TRANSLATE;
        ImGuizmo::MODE m_GuizmoMode      = ImGuizmo::MODE::LOCAL;

        bool   m_IsWindowHovered = false;
        Entity m_HoveredEntity   = {};
    };
} // namespace SnowLeopardEngine::Editor