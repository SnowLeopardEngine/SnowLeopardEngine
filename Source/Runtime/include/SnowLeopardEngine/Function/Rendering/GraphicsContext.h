#pragma once

#include "SnowLeopardEngine/Core/Base/Base.h"

namespace SnowLeopardEngine
{
    class GraphicsContext
    {
    public:
        virtual void Init()     = 0;
        virtual void Shutdown() = 0;

        virtual void SwapBuffers()               = 0;
        virtual void SetVSync(bool vsyncEnabled) = 0;

        static Ref<GraphicsContext> Create();

        bool IsSupportDSA() const { return m_SupportDSA; }

    protected:
        bool m_SupportDSA;
    };
} // namespace SnowLeopardEngine