#pragma once

#include "SnowLeopardEngine/Core/Base/Base.h"

namespace SnowLeopardEngine
{
    class GraphicsContext
    {
    public:
        void Init();
        void Shutdown();

        void SwapBuffers();
        void SetVSync(bool vsyncEnabled);

        bool IsSupportDSA() const { return m_SupportDSA; }

    private:
        int LoadGL();
        int GetMinMajor();
        int GetMinMinor();

    protected:
        bool m_SupportDSA;
    };
} // namespace SnowLeopardEngine