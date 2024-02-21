#pragma once

#include "SnowLeopardEngine/Core/Base/Base.h"
#include "SnowLeopardEngine/Function/Rendering/Pass/RenderDrawPass.h"
#include "SnowLeopardEngine/Function/Rendering/Pass/RenderSubDrawPass.h"

namespace SnowLeopardEngine
{
    class BaseRenderDrawPass : public RenderDrawPass
    {
    public:
        virtual void EndPass() override {}
        virtual void Draw() override;

    protected:
        std::vector<Ref<RenderSubDrawPass>> m_SubDrawPasses;
    };
} // namespace SnowLeopardEngine