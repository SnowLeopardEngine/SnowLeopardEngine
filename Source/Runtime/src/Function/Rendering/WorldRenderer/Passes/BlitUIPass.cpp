#include "SnowLeopardEngine/Function/Rendering/WorldRenderer/Passes/BlitUIPass.h"
#include "SnowLeopardEngine/Core/Base/Macro.h"
#include "SnowLeopardEngine/Function/Rendering/FrameGraph/FrameGraphHelper.h"
#include "SnowLeopardEngine/Function/Rendering/FrameGraph/FrameGraphTexture.h"
#include "SnowLeopardEngine/Function/Rendering/Pipeline/PipelineState.h"
#include "SnowLeopardEngine/Function/Rendering/ShaderCompiler.h"

#include <fg/Blackboard.hpp>
#include <fg/FrameGraph.hpp>

namespace SnowLeopardEngine
{
    BlitUIPass::BlitUIPass(RenderContext& rc) : m_RenderContext(rc)
    {
        auto vertResult = ShaderCompiler::Compile("Assets/Shaders/FullScreenTriangle.vert");
        SNOW_LEOPARD_CORE_ASSERT(vertResult.Success, "{0}", vertResult.Message);

        auto fragResult = ShaderCompiler::Compile("Assets/Shaders/BlitUIPass.frag");
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
                         .SetBlendState(0,
                                        {
                                            .Enabled   = true,
                                            .SrcColor  = BlendFactor::One,
                                            .DestColor = BlendFactor::Zero,
                                        })
                         .Build();
    }

    BlitUIPass::~BlitUIPass() { m_RenderContext.Destroy(m_Pipeline); }

    FrameGraphResource BlitUIPass::AddToGraph(FrameGraph& fg, FrameGraphResource target, FrameGraphResource source)
    {
        assert(target != source);

        fg.addCallbackPass(
            "Blit Pass",
            [&](FrameGraph::Builder& builder, auto&) {
                builder.read(source);

                target = builder.write(target);
            },
            [=, this](const auto&, FrameGraphPassResources& resources, void* ctx) {
                const auto          extent = resources.getDescriptor<FrameGraphTexture>(target).Extent;
                const RenderingInfo renderingInfo {
                    .Area             = {.Extent = extent},
                    .ColorAttachments = {{
                        .Image = getTexture(resources, target),
                    }},
                };
                auto&      rc          = *static_cast<RenderContext*>(ctx);
                const auto framebuffer = rc.BeginRendering(renderingInfo);
                rc.BindGraphicsPipeline(m_Pipeline)
                    .BindTexture(0, getTexture(resources, source))
                    .DrawFullScreenTriangle()
                    .EndRendering(framebuffer);
            });

        return target;
    }
} // namespace SnowLeopardEngine