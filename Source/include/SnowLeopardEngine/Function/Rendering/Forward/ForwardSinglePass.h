#pragma once

#include "SnowLeopardEngine/Function/Rendering/Pass/RenderDrawPass.h"
#include "SnowLeopardEngine/Function/Rendering/RHI/Shader.h"

namespace SnowLeopardEngine
{
    class ForwardSinglePass : public RenderDrawPass
    {
    public:
        virtual void BeginPass() override {}
        virtual void EndPass() override {}

        virtual void Draw() override final;

    private:
        Ref<Shader> m_Shader;
    };
} // namespace SnowLeopardEngine