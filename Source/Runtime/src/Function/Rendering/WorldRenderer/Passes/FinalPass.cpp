#include "SnowLeopardEngine/Function/Rendering/WorldRenderer/Passes/FinalPass.h"
#include "SnowLeopardEngine/Core/Base/Macro.h"
#include "SnowLeopardEngine/Core/Profiling/Profiling.h"
#include "SnowLeopardEngine/Function/Rendering/FrameGraph/FrameGraphHelper.h"
#include "SnowLeopardEngine/Function/Rendering/FrameGraph/FrameGraphTexture.h"
#include "SnowLeopardEngine/Function/Rendering/ShaderCompiler.h"
#include "SnowLeopardEngine/Function/Rendering/WorldRenderer/Resources/FrameData.h"
#include "SnowLeopardEngine/Function/Rendering/WorldRenderer/Resources/SceneColorData.h"

#include <fg/Blackboard.hpp>
#include <fg/FrameGraph.hpp>

namespace SnowLeopardEngine
{
    FinalPass::FinalPass(RenderContext& rc) : m_RenderContext(rc)
    {
        auto vertResult = ShaderCompiler::Compile("Assets/Shaders/FullScreenTriangle.vert");
        SNOW_LEOPARD_CORE_ASSERT(vertResult.Success, "{0}", vertResult.Message);

        auto fragResult = ShaderCompiler::Compile("Assets/Shaders/FinalPass.frag");
        SNOW_LEOPARD_CORE_ASSERT(fragResult.Success, "{0}", fragResult.Message);

        auto program = m_RenderContext.CreateGraphicsProgram(vertResult.ProgramCode, fragResult.ProgramCode);

        m_Pipeline = GraphicsPipeline::Builder {}
                         .SetShaderProgram(program)
                         .SetDepthStencil({
                             .DepthTest  = false,
                             .DepthWrite = false,
                         })
                         .SetRasterizerState({
                             .PolygonMode = PolygonMode::Fill,
                             .CullMode    = CullMode::Back,
                             .ScissorTest = false,
                         })
                         .Build();
    }

    FinalPass::~FinalPass() { m_RenderContext.Destroy(m_Pipeline); }

    void FinalPass::Compose(FrameGraph& fg, const FrameGraphBlackboard& blackboard)
    {
        const auto [frameUniform] = blackboard.get<FrameData>();

        auto output = blackboard.get<SceneColorData>().LDR;

        fg.addCallbackPass(
            "Final Composition Pass",
            [&](FrameGraph::Builder& builder, auto&) {
                builder.read(frameUniform);
                builder.read(output);
                builder.setSideEffect();
            },
            [=, this](const auto&, FrameGraphPassResources& resources, void* ctx) {
                NAMED_DEBUG_MARKER("Final Composition Pass");
                SNOW_LEOPARD_PROFILE_GL("Final Composition Pass");

                const auto extent = resources.getDescriptor<FrameGraphTexture>(output).Extent;
                auto&      rc     = *static_cast<RenderContext*>(ctx);

                rc.BeginRendering({.Extent = extent}, glm::vec4 {0.0f});
                rc.BindGraphicsPipeline(m_Pipeline)
                    .BindUniformBuffer(0, getBuffer(resources, frameUniform))
                    .BindTexture(0, getTexture(resources, output))
                    .DrawFullScreenTriangle();
            });
    }
} // namespace SnowLeopardEngine