#pragma once

#include "SnowLeopardEditor/PanelBase.h"
#include "SnowLeopardEngine/Core/Base/Base.h"
#include "SnowLeopardEngine/Function/Rendering/RHI/FrameBuffer.h"

namespace SnowLeopardEngine::Editor
{
    class ViewportPanel final : public PanelBase
    {
    public:
        virtual void Init() override final;
        virtual void OnFixedTick() override final;
        virtual void OnTick(float deltaTime) override final;

    private:
        Ref<FrameBuffer> m_RenderTarget = nullptr;
        glm::vec2        m_ViewportSize = {1024, 1024};
    };
} // namespace SnowLeopardEngine::Editor