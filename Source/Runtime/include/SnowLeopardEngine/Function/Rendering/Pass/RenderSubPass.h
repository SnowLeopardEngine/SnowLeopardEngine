#pragma once

#include "SnowLeopardEngine/Function/Rendering/RHI/Shader.h"

namespace SnowLeopardEngine
{
    class RenderPass;

    class RenderSubPass
    {
    public:
        virtual ~RenderSubPass() = default;

        virtual void BeginSubPass() = 0;
        virtual void EndSubPass()   = 0;

        void BindOwnerPass(RenderPass* ownerPass) { m_OwnerPass = ownerPass; }

    protected:
        RenderPass* m_OwnerPass;
        Ref<Shader> m_Shader;
    };
} // namespace SnowLeopardEngine