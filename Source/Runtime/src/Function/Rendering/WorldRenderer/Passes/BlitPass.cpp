#include "SnowLeopardEngine/Function/Rendering/WorldRenderer/Passes/BlitPass.h"
#include "SnowLeopardEngine/Core/Base/Macro.h"
#include "SnowLeopardEngine/Core/Profiling/Profiling.h"
#include "SnowLeopardEngine/Function/Rendering/FrameGraph/FrameGraphHelper.h"
#include "SnowLeopardEngine/Function/Rendering/FrameGraph/FrameGraphTexture.h"
#include "SnowLeopardEngine/Function/Rendering/Pipeline/PipelineState.h"
#include "SnowLeopardEngine/Function/Rendering/RenderContext.h"
#include "SnowLeopardEngine/Function/Rendering/ShaderCompiler.h"
#include "SnowLeopardEngine/Function/Rendering/WorldRenderer/Resources/GBufferData.h"

#include <fg/Blackboard.hpp>
#include <fg/FrameGraph.hpp>

namespace SnowLeopardEngine
{
    BlitPass::BlitPass(RenderContext& rc) : m_RenderContext(rc)
    {
        auto vertResult = ShaderCompiler::Compile("Assets/Shaders/FullScreenTriangle.vert");
        SNOW_LEOPARD_CORE_ASSERT(vertResult.Success, "{0}", vertResult.Message);

        auto fragResult = ShaderCompiler::Compile("Assets/Shaders/BlitPass.frag");
        SNOW_LEOPARD_CORE_ASSERT(fragResult.Success, "{0}", fragResult.Message);

        auto program = m_RenderContext.CreateGraphicsProgram(vertResult.ProgramCode, fragResult.ProgramCode);

        m_Pipeline = GraphicsPipeline::Builder {}
                         .SetShaderProgram(program)
                         .SetDepthStencil({
                             .DepthTest  = true,
                             .DepthWrite = false,
                         })
                         .SetRasterizerState({
                             .PolygonMode = PolygonMode::Fill,
                             .CullMode    = CullMode::Back,
                             .ScissorTest = false,
                         })
                         .Build();
    }

    BlitPass::~BlitPass() { m_RenderContext.Destroy(m_Pipeline); }

    FrameGraphResource BlitPass::AddToGraph(FrameGraph&           fg,
                                            FrameGraphBlackboard& blackboard,
                                            FrameGraphResource    target,
                                            FrameGraphResource    source)
    {
        assert(target != source);

        auto&       gBuffer = blackboard.get<GBufferData>();
        const auto& extent  = fg.getDescriptor<FrameGraphTexture>(target).Extent;

        struct Data
        {
            FrameGraphResource Output;
        };
        const auto& pass = fg.addCallbackPass<Data>(
            "Blit Pass",
            [&](FrameGraph::Builder& builder, Data& data) {
                builder.read(source);
                builder.read(target);
                builder.read(gBuffer.Depth);

                data.Output =
                    builder.create<FrameGraphTexture>("Blitted", {.Extent = extent, .Format = PixelFormat::RGB8_UNorm});
                data.Output = builder.write(data.Output);
            },
            [=, this](const Data& data, FrameGraphPassResources& resources, void* ctx) {
                NAMED_DEBUG_MARKER("Blit Pass");
                SNOW_LEOPARD_PROFILE_GL("Blit Pass");

                const RenderingInfo renderingInfo {.Area             = {.Extent = extent},
                                                   .ColorAttachments = {{.Image = getTexture(resources, data.Output)}},
                                                   .DepthAttachment =
                                                       AttachmentInfo {.Image = getTexture(resources, gBuffer.Depth)}};

                auto&      rc          = *static_cast<RenderContext*>(ctx);
                const auto framebuffer = rc.BeginRendering(renderingInfo);
                rc.BindGraphicsPipeline(m_Pipeline)
                    .BindTexture(0, getTexture(resources, source))
                    .BindTexture(1, getTexture(resources, target))
                    .DrawFullScreenTriangle()
                    .EndRendering(framebuffer);
            });

        return pass.Output;
    }
} // namespace SnowLeopardEngine