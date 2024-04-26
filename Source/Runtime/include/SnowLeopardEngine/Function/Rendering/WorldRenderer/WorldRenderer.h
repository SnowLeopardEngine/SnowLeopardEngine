#pragma once

#include "SnowLeopardEngine/Function/Rendering/WorldRenderer/Passes/GBufferPass.h"

namespace SnowLeopardEngine
{
    class WorldRenderer
    {
    public:
        WorldRenderer();

        void CreatePasses();

        void FilterRenderables();

        void RenderFrame();

    private:
        // -------- Passes --------

        Scope<RenderContext> m_RenderContext;
        Scope<GBufferPass>   m_GBufferPass;

        // States
    };
} // namespace SnowLeopardEngine