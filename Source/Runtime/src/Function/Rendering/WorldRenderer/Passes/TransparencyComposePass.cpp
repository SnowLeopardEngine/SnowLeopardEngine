#include "SnowLeopardEngine/Function/Rendering/WorldRenderer/Passes/TransparencyComposePass.h"
#include "SnowLeopardEngine/Core/Base/Macro.h"
#include "SnowLeopardEngine/Core/Profiling/Profiling.h"
#include "SnowLeopardEngine/Function/Rendering/FrameGraph/FrameGraphHelper.h"
#include "SnowLeopardEngine/Function/Rendering/FrameGraph/FrameGraphTexture.h"
#include "SnowLeopardEngine/Function/Rendering/ShaderCompiler.h"
#include "SnowLeopardEngine/Function/Rendering/WorldRenderer/Resources/WeightedBlendedData.h"

#include <fg/Blackboard.hpp>
#include <fg/FrameGraph.hpp>

namespace SnowLeopardEngine
{
    TransparencyComposePass::TransparencyComposePass(RenderContext& rc) : m_RenderContext(rc)
    {
        auto vertResult = ShaderCompiler::Compile("Assets/Shaders/FullScreenTriangle.vert");
        SNOW_LEOPARD_CORE_ASSERT(vertResult.Success, "{0}", vertResult.Message);

        auto fragResult = ShaderCompiler::Compile("Assets/Shaders/TransparencyComposePass.frag");
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
                                            .SrcColor  = BlendFactor::SrcColor,
                                            .DestColor = BlendFactor::OneMinusSrcColor,
                                            .SrcAlpha  = BlendFactor::SrcAlpha,
                                            .DestAlpha = BlendFactor::OneMinusSrcAlpha,
                                        })
                         .Build();
    }

    TransparencyComposePass::~TransparencyComposePass() { m_RenderContext.Destroy(m_Pipeline); }

    FrameGraphResource
    TransparencyComposePass::AddToGraph(FrameGraph& fg, FrameGraphBlackboard& blackboard, FrameGraphResource target)
    {
        const auto& weightedBlended = blackboard.get<WeightedBlendedData>();
        const auto  extent          = fg.getDescriptor<FrameGraphTexture>(target).Extent;

        fg.addCallbackPass(
            "TransparencyCompose Pass",
            [&](FrameGraph::Builder& builder, auto&) {
                builder.read(weightedBlended.Accum);
                builder.read(weightedBlended.Reveal);

                target = builder.write(target);
            },
            [=, this](auto&, FrameGraphPassResources& resources, void* ctx) {
                NAMED_DEBUG_MARKER("TransparencyCompose Pass");
                SNOW_LEOPARD_PROFILE_GL("TransparencyCompose Pass");

                const RenderingInfo renderingInfo {
                    .Area             = {.Extent = extent},
                    .ColorAttachments = {{
                        .Image = getTexture(resources, target),
                    }},
                };

                auto&      rc          = *static_cast<RenderContext*>(ctx);
                const auto framebuffer = rc.BeginRendering(renderingInfo);
                rc.BindGraphicsPipeline(m_Pipeline)
                    .BindTexture(0, getTexture(resources, weightedBlended.Accum))
                    .BindTexture(1, getTexture(resources, weightedBlended.Reveal))
                    .DrawFullScreenTriangle()
                    .EndRendering(framebuffer);
            });

        return target;
    }
} // namespace SnowLeopardEngine