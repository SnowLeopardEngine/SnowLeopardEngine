#include "SnowLeopardEngine/Function/Rendering/WorldRenderer/WorldRenderer.h"
#include "Blackboard.hpp"

namespace SnowLeopardEngine
{
    WorldRenderer::WorldRenderer() { m_RenderContext = CreateScope<RenderContext>(); }

    void WorldRenderer::CreatePasses() { m_GBufferPass = CreateScope<GBufferPass>(*m_RenderContext); }

    void WorldRenderer::FilterRenderables() {}

    void WorldRenderer::RenderFrame()
    {
        FrameGraph           fg;
        FrameGraphBlackboard blackboard;

        m_GBufferPass->AddToGraph(fg, blackboard);

        fg.compile();
        fg.execute(m_RenderContext.get());

#ifndef NDEBUG
        // Built in graphviz writer.
        std::ofstream {"DebugFrameGraph.dot"} << fg;
#endif
    }
} // namespace SnowLeopardEngine