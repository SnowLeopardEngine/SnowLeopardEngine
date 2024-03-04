#pragma once

#include "SnowLeopardEngine/Function/Rendering/Pass/BaseRenderDrawPass.h"
#include "SnowLeopardEngine/Function/Rendering/RHI/FrameBuffer.h"

namespace SnowLeopardEngine
{
    class ForwardSinglePass : public BaseRenderDrawPass
    {
    public:
        ForwardSinglePass();

        Ref<FrameBuffer> GetShadowDepthBuffer() const { return m_DepthBuffer; }

    private:
        Ref<FrameBuffer> m_DepthBuffer = nullptr;
    };
} // namespace SnowLeopardEngine